

#define STB_IMAGE_IMPLEMENTATION
#include "stb_image.h"

#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <iomanip>
#include <stdlib.h>
#include <sstream>

#include "immintrin.h"
#include "tbb/parallel_for.h"
#include "TileManager.hpp"
#include "Rasterizer.hpp"
#include "camera.hpp"
#include "Scene.hpp"

int main()
{
	bool cameraRotatingRight = false, cameraRotatingLeft = false, cameraRotatingUp = false, cameraRotatingDown = false;
	bool cameraPanForward = false, cameraPanBackwards = false, cameraPanLeft = false, cameraPanRight = false;

	//recommend near value of 2, might have artifacts if < 1
	Camera cam(1, 5, 2, 1000, 60);

	//height & width values must be divisible by 16
	//no aspect ratio scaling as of yet!!
	Rasterizer rasterizer = Rasterizer(1024, 1024, cam.fov, cam.cNear, cam.cFar);
	sf::RenderWindow window(sf::VideoMode(rasterizer.wWidth, rasterizer.wHeight), "3D Render");

	sf::Clock clock = sf::Clock::Clock();
	sf::Time lastTime = clock.getElapsedTime();
	sf::Time curTime;
	sf::Font font;
	font.loadFromFile("arial.ttf");
	float fps;

	int numThreads = std::thread::hardware_concurrency() - 1;
	if (numThreads <= 0)
		numThreads = 1;
	tbb::task_group threadPool;

	Scene scene;
	scene.loadScene("sponza.obj");

	int numTiles = 8;

	sf::Event event;
	while (window.isOpen())
	{
		//Handle keyboard input
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Right) cameraRotatingRight = true;
				else if (event.key.code == sf::Keyboard::Left) cameraRotatingLeft = true;
				else if (event.key.code == sf::Keyboard::Up) cameraRotatingUp = true;
				else if (event.key.code == sf::Keyboard::Down) cameraRotatingDown = true;
				else if (event.key.code == sf::Keyboard::W) cameraPanForward = true;
				else if (event.key.code == sf::Keyboard::S) cameraPanBackwards = true;
				else if (event.key.code == sf::Keyboard::A) cameraPanLeft = true;
				else if (event.key.code == sf::Keyboard::D) cameraPanRight = true;
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Right) cameraRotatingRight = false;
				else if (event.key.code == sf::Keyboard::Left) cameraRotatingLeft = false;
				else if (event.key.code == sf::Keyboard::Up) cameraRotatingUp = false;
				else if (event.key.code == sf::Keyboard::Down) cameraRotatingDown = false;
				else if (event.key.code == sf::Keyboard::W) cameraPanForward = false;
				else if (event.key.code == sf::Keyboard::S) cameraPanBackwards = false;
				else if (event.key.code == sf::Keyboard::A) cameraPanLeft = false;
				else if (event.key.code == sf::Keyboard::D) cameraPanRight = false;
			}
		}

		/*
		Since our clipping algorithm can produce new triangles that we want to discard after each frame, we add an additional vector
		called trisToDelete which has pointers to all triangles that don't originate from the original mesh, this way we can easily
		loop through and delete each triangle after the frame ends. Each vector is actually a vector of vectors of triangles, which
		allows us to easily split up our triangles into smaller chunks which can then be sent to individual threads safely and quickly.
		*/
		std::vector<std::vector<Triangle*>> triangleLists(numThreads, std::vector<Triangle*>());
		std::vector<std::vector<Triangle*>> trisToDelete(numThreads, std::vector<Triangle*>());



		//Update camera position(this sets a transformation matrix in cam.camMatrix)
		cam.updateCamera(cameraRotatingLeft, cameraRotatingRight, cameraRotatingUp, cameraRotatingDown,
			cameraPanForward, cameraPanBackwards, cameraPanLeft, cameraPanRight);

		/*
		Our camera handles conversion to view space based off camera movement. It also clips triangles
		against the camera's near plane and culls triangles that are outside the camera's view frustum
		*/
		int increment = scene.sceneData.size() / numThreads;
		for (int i = 0; i < numThreads; ++i)
		{
			int start = i * increment;
			int end = (i >= numThreads - 1) ? scene.sceneData.size() : start + increment;

			threadPool.run([&, start, end, i]
				{
					for (int j = start; j < end; ++j)
					{
						if (!cam.checkIfTriangleCulled(scene.sceneData[j]))
						{
							cam.transformToViewSpace(scene.sceneData[j]);
							cam.clipTriangleNear(scene.sceneData[j], triangleLists[i], trisToDelete[i]);
						}
					}
				});
		}
		threadPool.wait();


		/*
		Our rasterizer will convert triangles from view space to projected space while calculating any needed data
		along the way. It also rasters each pixel to the screen after the tile manager runs.
		*/
		for (auto& list : triangleLists)
		{
			threadPool.run([&]
				{
					for (auto& tri : list)
					{
						rasterizer.project_triangle(*tri);
						rasterizer.calculateBoundingBox(*tri);
						rasterizer.calculateVertexData(*tri);
					}
				});
		}
		threadPool.wait();


		/*
		Tile Manager initiatlizes each tile and stores all triangles that potentially overlap each tile.
		*/
		TileManager tileManager(numTiles, rasterizer.wWidth, rasterizer.wHeight);
		for (auto& list : triangleLists)
		{
			tileManager.binTriangles(list);
		}

		//Our pixel buffer and depth buffers
		sf::Uint8* pixelBuffer = new sf::Uint8[rasterizer.wWidth * rasterizer.wHeight * 4];
		std::vector<std::vector<float>> zBuffer((float)rasterizer.wWidth, std::vector<float>(rasterizer.wHeight, FLT_MAX));

		//Render pixels to screen
		for (auto& row : tileManager.tiles)
		{
			for (auto& tile : row)
				threadPool.run([&] {rasterizer.rasterTile(tile, zBuffer, pixelBuffer); });
		}
		threadPool.wait();


		//Compute number of triangles currently being processed on screen
		int numTrisBeingDrawn = 0;
		for (auto& list : triangleLists)
			numTrisBeingDrawn += list.size();


		//clear previous frame
		window.clear();

		//setup to display fps counter
		curTime = clock.getElapsedTime();
		fps = 1.0f / (curTime.asSeconds() - lastTime.asSeconds());
		sf::Text frames(std::to_string((int)fps) + " N= " + std::to_string((int)numTrisBeingDrawn), font, 50);
		frames.setFillColor(sf::Color::Cyan);
		lastTime = curTime;


		//using SFML to display pixels on screen
		sf::Image image;
		image.create(rasterizer.wWidth, rasterizer.wHeight, pixelBuffer);

		sf::Texture texture;
		texture.loadFromImage(image);

		sf::Sprite sprite(texture);

		window.draw(sprite);
		window.draw(frames);
		window.display();

		delete[] pixelBuffer;
		for (auto& list : trisToDelete)
			for (auto& ptr : list)
				delete ptr;
	}
	return 0;
}