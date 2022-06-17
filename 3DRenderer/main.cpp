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

	bool camera_rotating_right = false, camera_rotating_left = false, camera_rotating_up = false, camera_rotating_down = false;
	bool camera_pan_forward = false, camera_pan_backwards = false, camera_pan_left = false, camera_pan_right = false;
	Camera cam(1, 5);

	//height & width values must be divisible by 16
	//recommend near value of 2, might have artifacts if < 1
	Rasterizer rasterizer = Rasterizer(1024, 1024, 2, 1000, 60);
	sf::RenderWindow window(sf::VideoMode(rasterizer.wWidth, rasterizer.wHeight), "3D Render");

	sf::Clock clock = sf::Clock::Clock();
	sf::Time last_time = clock.getElapsedTime();
	sf::Time cur_time;
	sf::Font font;
	font.loadFromFile("arial.ttf");
	float fps;
	
	int numThreads = std::thread::hardware_concurrency() - 1;
	if (numThreads <= 0)
		numThreads = 1;
	tbb::task_group g;

	Scene scene;
	scene.loadScene("sponza.obj");

	std::vector<std::string> names;
	for (std::pair<std::string, sf::Uint8*> name : scene.textureData)
	{
		names.push_back(name.first);
	}

	int bufferSize = rasterizer.wWidth * rasterizer.wHeight * 4;
	int numTiles = 8;

	while (window.isOpen())
	{
		//Handle keyboard input
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Right) camera_rotating_right = true;
				else if (event.key.code == sf::Keyboard::Left) camera_rotating_left = true;
				else if (event.key.code == sf::Keyboard::Up) camera_rotating_up = true;
				else if (event.key.code == sf::Keyboard::Down) camera_rotating_down = true;
				else if (event.key.code == sf::Keyboard::W) camera_pan_forward = true;
				else if (event.key.code == sf::Keyboard::S) camera_pan_backwards = true;
				else if (event.key.code == sf::Keyboard::A) camera_pan_left = true;
				else if (event.key.code == sf::Keyboard::D) camera_pan_right = true;
			}
			else if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Right) camera_rotating_right = false;
				else if (event.key.code == sf::Keyboard::Left) camera_rotating_left = false;
				else if (event.key.code == sf::Keyboard::Up) camera_rotating_up = false;
				else if (event.key.code == sf::Keyboard::Down) camera_rotating_down = false;
				else if (event.key.code == sf::Keyboard::W) camera_pan_forward = false;
				else if (event.key.code == sf::Keyboard::S) camera_pan_backwards = false;
				else if (event.key.code == sf::Keyboard::A) camera_pan_left = false;
				else if (event.key.code == sf::Keyboard::D) camera_pan_right = false;
			}

		}

		//DECLARE/RESET our buffers
		sf::Uint8* buffer = new sf::Uint8[bufferSize];
		std::vector<std::vector<float>> z_buffer((float)rasterizer.wWidth, std::vector<float>(rasterizer.wHeight, FLT_MAX));

		/*
		Since our clipping algorithm can produce new triangles that we want to discard after each frame, we add an additional vector
		called trisToDelete which has pointers to all triangles that don't originate from the original mesh, this way we can easily
		loop through and delete each triangle after the frame ends. Each vector is actually a vector of vectors of triangles, which
		allows us to easily split up our triangles into smaller chunks which can then be sent to individual threads safely and quickly.
		*/
		std::vector<std::vector<Triangle*>> triangleLists(numThreads, std::vector<Triangle*>());
		std::vector<std::vector<Triangle*>> trisToDelete(numThreads, std::vector<Triangle*>());



		//Update camera position(this sets a transformation matrix in cam.camMatrix)
		cam.updateCamera(camera_rotating_left, camera_rotating_right, camera_rotating_up, camera_rotating_down,
			camera_pan_forward, camera_pan_backwards, camera_pan_left, camera_pan_right, rasterizer);

		/*
		Our camera handles conversion to view space based off camera movement. It also clips triangles 
		against the camera's near plane and culls triangles that are outside the camera's view frustum
		*/
		int increment = scene.sceneData.size() / numThreads;
		for (int i = 0; i < numThreads; ++i)
		{
			int start = i * increment;
			int end = (i >= numThreads - 1) ? scene.sceneData.size() : start + increment;

			g.run([&, start, end, i] 
			{
				for (int j = start; j < end; ++j) 
				{
					if (!cam.checkIfTriangleCulled(scene.sceneData[j]))
					{
						cam.transformToViewSpace(scene.sceneData[j]);
						cam.clipTriangleNear(scene.sceneData[j], triangleLists[i], trisToDelete[i], rasterizer.cNear);
					}
				}
			});
		}
		g.wait();


		/*
		Our rasterizer will convert triangles from view space to projected space while calculating any needed data
		along the way. It also rasters each pixel to the screen after the tile manager runs.
		*/
		for (auto& list : triangleLists)
		{
			g.run([&]
			{
				for (auto& tri : list)
				{
					rasterizer.project_triangle(*tri, rasterizer.pMat);
					rasterizer.calculateBoundingBox(*tri);
					rasterizer.calculateVertexData(*tri);
				}
			});
		}
		g.wait();


		/*
		Tile Manager initiatlizes each tile and stores all triangles that potentially overlap each tile.
		*/
		TileManager tileManager(numTiles, rasterizer.wWidth, rasterizer.wHeight);
		for (auto& list : triangleLists)
		{
			tileManager.binTriangles(list);
		}

		//Render pixels to screen
		for (auto& row : tileManager.tiles)
		{
			for (auto& tile : row)
				g.run([&] {rasterizer.rasterTile(tile, z_buffer, buffer); });
		}
		g.wait();


		//Compute number of triangles currently being processed on screen
		int numTrisBeingDrawn = 0;
		for (auto& list : triangleLists)
			numTrisBeingDrawn += list.size();


		//clear previous frame
		window.clear();

		//setup to display fps counter
		cur_time = clock.getElapsedTime();
		fps = 1.0f / (cur_time.asSeconds() - last_time.asSeconds());
		sf::Text frames(std::to_string((int)fps), font, 50);
		frames.setFillColor(sf::Color::Cyan);
		last_time = cur_time;


		//setup display triangle counter
		sf::Text numTrisText("N=" + std::to_string((int)numTrisBeingDrawn), font, 50);
		numTrisText.setPosition(sf::Vector2f(100, 0));
		numTrisText.setFillColor(sf::Color::Cyan);


		sf::Image image;
		image.create(rasterizer.wWidth, rasterizer.wHeight, buffer);

		sf::Texture texture;
		texture.loadFromImage(image);

		sf::Sprite sprite;
		sprite.setTexture(texture);

		window.draw(sprite);
		window.draw(frames);
		window.draw(numTrisText);
		window.display();

		delete[] buffer;
		for (auto& list : trisToDelete)
			for (auto& ptr : list)
				delete ptr;
	}
	return 0;
}