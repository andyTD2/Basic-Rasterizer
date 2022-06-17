#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <iomanip>
#include "Rasterizer.hpp"
#include <stdlib.h>
#include "immintrin.h"
#include "camera.hpp"
#include <sstream>
#include <mutex>
#include "Scene.hpp"
#include "tbb/parallel_for.h"
#include <tbb/global_control.h>
#include <time.h>
#include <thread>
#include "TileManager.hpp"


int main()
{
	//height & width values must be divisible by 16
	//recommend near value of 2, expect articfacts if < 1
	Rasterizer rasterizer = Rasterizer(1024, 1024, 2, 1000, 60);
	sf::RenderWindow window(sf::VideoMode(rasterizer.w_width, rasterizer.w_height), "3D Render");

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

	bool camera_rotating_right = false, camera_rotating_left = false, camera_rotating_up = false, camera_rotating_down = false;
	bool camera_pan_forward = false, camera_pan_backwards = false, camera_pan_left = false, camera_pan_right = false;
	Camera cam(1, 5);

	int bufferSize = rasterizer.w_width * rasterizer.w_height * 4;
	int numTiles = 8;

	while (window.isOpen())
	{
		clock_t total = std::clock();

		//DECLARE/RESET our buffers
		sf::Uint8* buffer = new sf::Uint8[bufferSize];
		std::vector<std::vector<float>> z_buffer((float)rasterizer.w_width, std::vector<float>(rasterizer.w_height, FLT_MAX));


		//Handle keyboard input
		sf::Event event;
		while (window.pollEvent(event))
		{
			if (event.type == sf::Event::Closed)
				window.close();

			if (event.type == sf::Event::KeyPressed)
			{
				if (event.key.code == sf::Keyboard::Right) camera_rotating_right = true;
				if (event.key.code == sf::Keyboard::Left) camera_rotating_left = true;
				if (event.key.code == sf::Keyboard::Up) camera_rotating_up = true;
				if (event.key.code == sf::Keyboard::Down) camera_rotating_down = true;
				if (event.key.code == sf::Keyboard::W) camera_pan_forward = true;
				if (event.key.code == sf::Keyboard::S) camera_pan_backwards = true;
				if (event.key.code == sf::Keyboard::A) camera_pan_left = true;
				if (event.key.code == sf::Keyboard::D) camera_pan_right = true;
			}
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Right) camera_rotating_right = false;
				if (event.key.code == sf::Keyboard::Left) camera_rotating_left = false;
				if (event.key.code == sf::Keyboard::Up) camera_rotating_up = false;
				if (event.key.code == sf::Keyboard::Down) camera_rotating_down = false;
				if (event.key.code == sf::Keyboard::W) camera_pan_forward = false;
				if (event.key.code == sf::Keyboard::S) camera_pan_backwards = false;
				if (event.key.code == sf::Keyboard::A) camera_pan_left = false;
				if (event.key.code == sf::Keyboard::D) camera_pan_right = false;
			}

		}

		//Update camera position(this sets a matrix in cam.camMatrix we can use to multiply our objects against later)
		cam.updateCamera(camera_rotating_left, camera_rotating_right, camera_rotating_up, camera_rotating_down,
			camera_pan_forward, camera_pan_backwards, camera_pan_left, camera_pan_right, rasterizer);

		std::vector<std::vector<Triangle*>> triangleLists(numThreads, std::vector<Triangle*>());
		std::vector<std::vector<Triangle*>> trisToDelete(numThreads, std::vector<Triangle*>());


		std::clock_t t;
		t = std::clock();
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
						rasterizer.clip_triangle_near(scene.sceneData[j], triangleLists[i], trisToDelete[i]);
					}
				}
			});
		}
		g.wait();
		t = std::clock() - t;
		std::cout << "viewspace and clipping: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;



		t = std::clock();
		for (int i = 0; i < triangleLists.size(); ++i)
		{
			g.run([&, i]
			{
				for (auto& tri : triangleLists[i])
				{
					rasterizer.project_triangle(*tri, rasterizer.p_mat);
				}
			});
		}
		g.wait();
		t = std::clock() - t;
		std::cout << "projection: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;


		t = std::clock();
		for (auto& list : triangleLists)
		{
			g.run([&] {
				for (auto& tri : list)
				{
					tri->bLeft = std::max(0.0f, std::min({ tri->projVerts[0].x, tri->projVerts[1].x, tri->projVerts[2].x, (float)rasterizer.w_width - 1 }));
					tri->bTop = std::max(0.0f, std::min({ tri->projVerts[0].y, tri->projVerts[1].y, tri->projVerts[2].y, (float)rasterizer.w_height - 1 }));
					tri->bRight = std::min((float)rasterizer.w_width - 1, std::max({ tri->projVerts[0].x, tri->projVerts[1].x, tri->projVerts[2].x, 0.0f }));
					tri->bBot = std::min((float)rasterizer.w_height - 1, std::max({ tri->projVerts[0].y, tri->projVerts[1].y, tri->projVerts[2].y, 0.0f }));

					tri->vertexDepth[0] = 1 / tri->transVerts[0].z;
					tri->vertexDepth[1] = 1 / tri->transVerts[1].z;
					tri->vertexDepth[2] = 1 / tri->transVerts[2].z;

					tri->triangleTexture = scene.textureData.find(tri->associatedMtl)->second;
					tri->tWidth = scene.widthMap.find(tri->associatedMtl)->second;
					tri->tHeight = scene.heightMap.find(tri->associatedMtl)->second;

					tri->tx[0] = tri->tCoords[0].x / tri->transVerts[0].z; tri->tx[1] = tri->tCoords[1].x / tri->transVerts[1].z; tri->tx[2] = tri->tCoords[2].x / tri->transVerts[2].z;
					tri->ty[0] = tri->tCoords[0].y / tri->transVerts[0].z; tri->ty[1] = tri->tCoords[1].y / tri->transVerts[1].z; tri->ty[2] = tri->tCoords[2].y / tri->transVerts[2].z;
					tri->area = func::edge_f(vec2(tri->projVerts[0].x, tri->projVerts[0].y), tri->projVerts[1], tri->projVerts[2]);
				}
			});
		}
		g.wait();

		TileManager tileManager(numTiles, rasterizer.w_width, rasterizer.w_height);

		for (auto& list : triangleLists)
		{
			tileManager.binTriangles(list);
		}

		t = std::clock() - t;
		std::cout << "binning: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;

		t = std::clock();
		for (auto& row : tileManager.tiles)
		{
			for (auto& tile : row)
				//g.run([&] {bar(tile, z_buffer, rasterizer, buffer); });
				g.run([&] {rasterizer.rasterTile(tile, z_buffer, buffer); });
		}

		g.wait();
		t = std::clock() - t;
		std::cout << "rasterization: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;


		int numTrisBeingDrawn = 0;


		//fps counter
		cur_time = clock.getElapsedTime();
		fps = 1.0f / (cur_time.asSeconds() - last_time.asSeconds());
		sf::Text frames(std::to_string((int)fps), font, 50);
		frames.setFillColor(sf::Color::Cyan);
		last_time = cur_time;

		sf::Text numTrisText("N=" + std::to_string((int)numTrisBeingDrawn), font, 50);
		numTrisText.setPosition(sf::Vector2f(100, 0));
		numTrisText.setFillColor(sf::Color::Cyan);

		window.clear();
		sf::Image image;


		image.create(rasterizer.w_width, rasterizer.w_height, buffer);
		delete[] buffer;
	
		t = std::clock();
		for (auto& list : trisToDelete)
			for (auto& ptr : list)
				delete ptr;
		t = std::clock() - t;
		std::cout << "deletion: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;

		t = std::clock();
		sf::Texture texture;
		texture.loadFromImage(image);

		sf::Sprite sprite;
		sprite.setTexture(texture);

		window.draw(sprite);
		window.draw(frames);
		window.draw(numTrisText);

		window.display();
		//system("Pause");
		t = std::clock() - t;
		std::cout << "display: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;
		
		total = std::clock() - total;
		std::cout << "total: " << (float)total / CLOCKS_PER_SEC << " seconds" << std::endl;

	}
	return 0;
}