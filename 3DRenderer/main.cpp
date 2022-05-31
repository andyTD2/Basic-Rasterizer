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



void foo(std::vector<Triangle>& preparedTriangles, Rasterizer& rasterizer, std::vector<std::vector<float>>& z_buffer, const Scene& scene, sf::Uint8*& buffer)
{

	for (auto& triangle : preparedTriangles)
	{
		sf::Vector3f proj_verts[3];
		rasterizer.project_triangle(triangle.verts, rasterizer.p_mat, proj_verts);

		float va = 1 / triangle.verts[0].z;
		float vb = 1 / triangle.verts[1].z;
		float vc = 1 / triangle.verts[2].z;

		int bLeft = std::min({ proj_verts[0].x, proj_verts[1].x, proj_verts[2].x });
		int bTop = std::min({ proj_verts[0].y, proj_verts[1].y, proj_verts[2].y });
		int bRight = std::max({ proj_verts[0].x, proj_verts[1].x, proj_verts[2].x });
		int bBot = std::max({ proj_verts[0].y, proj_verts[1].y, proj_verts[2].y });

		//std::cout << triangle.associatedMtl << std::endl;
		bool texttest = false;
		std::unordered_map<std::string, sf::Uint8*>::const_iterator got = scene.textureData.find(triangle.associatedMtl);
		if (got == scene.textureData.end())
		{
			//std::cout << triangle.associatedMtl << std::endl;
			//std::cout << "DID NOT FIND TEXTURE\n";
			texttest = true;
		}
		else
			texttest = false;
		sf::Uint8* triangleTexture = scene.textureData.find(triangle.associatedMtl)->second;
		int tWidth = scene.widthMap.find(triangle.associatedMtl)->second;
		int tHeight = scene.heightMap.find(triangle.associatedMtl)->second;

		//std::cout << "width: " << tWidth << ", height: " << tHeight << std::endl;
		if (!texttest && bLeft < rasterizer.w_width && bRight > -1 && bTop < rasterizer.w_height - 1 && bBot > -1)
		{
			bLeft = std::max(0, std::min(bLeft, rasterizer.w_width - 1));
			bTop = std::max(0, std::min(bTop, rasterizer.w_height - 1));
			bRight = std::min(rasterizer.w_width - 1, std::max(bRight, 0));
			bBot = std::min(rasterizer.w_height - 1, std::max(bBot, 0));

			float area = func::edge_f(sf::Vector2f(proj_verts[0].x, proj_verts[0].y), proj_verts[1], proj_verts[2]);


			sf::Vector2f pixel(std::max(bLeft, 0) + .5, std::max(0, bTop) + .5);

			for (int i = bTop; i < bBot; i += 1)
			{

				for (int j = bLeft; j < bRight; j += 1)
				{
					sf::Vector2f pixel(j, i);
					float w0 = func::edge_f(pixel, proj_verts[1], proj_verts[2]);
					float w1 = func::edge_f(pixel, proj_verts[2], proj_verts[0]);
					float w2 = func::edge_f(pixel, proj_verts[0], proj_verts[1]);
					//USE THE FIRST IF STATEMENT IF YOU DONT WANT BACKFACE CULLING
					//if ((one[k] >= 0 && two[k] >= 0 && three[k] >= 0) || ((one[k] < 0 && two[k] < 0 && three[k] < 0)))
					if ((w0 <= 0 && w1 <= 0 && w2 <= 0) || (w0 >= 0 && w1 >= 0 && w2 >= 0))
					{
						float b0t = w0 / (area);
						float b1t = w1 / (area);
						float b2t = w2 / (area);
					
						float z = b0t * va +
							b1t * vb +
							b2t * vc;

						sf::Vector2f texel;

						float inv_z = 1 / z;
						if (inv_z < z_buffer[j][i])
						{
							//z_buffer[j][i] = inv_z;

							
							int index = (j + i * rasterizer.w_width) * 4;
														
							float w = ((1 / triangle.verts[0].z) * b0t) +
								((1 / triangle.verts[1].z) * b1t) +
								((1 / triangle.verts[2].z) * b2t);

							float u = ((triangle.tCoords[0].x / triangle.verts[0].z) * b0t) +
								((triangle.tCoords[1].x / triangle.verts[1].z) * b1t) +
								((triangle.tCoords[2].x / triangle.verts[2].z) * b2t);

							float v = ((triangle.tCoords[0].y / triangle.verts[0].z) * b0t) +
								((triangle.tCoords[1].y / triangle.verts[1].z) * b1t) +
								((triangle.tCoords[2].y / triangle.verts[2].z) * b2t);


							texel.x = u / w;
							texel.y = v / w;
							float whatever;
							if (texel.x > 1)
								texel.x--;

							if (texel.y > 1)
								texel.y--;


							if (texel.x < 0)
							{
								texel.x *= -1;
							}
							if (texel.y < 0)
							{
								texel.y *= -1;
							}
							if (texel.x > 1.0f)
							{
								texel.x -= (int)texel.x;
							}
							if (texel.y > 1.0f)
							{
								texel.y -= (int)texel.y;
							}

							texel.x *= (tWidth - 1);
							texel.y *= (tHeight - 1);


							int index2 = ((int)texel.x + (int)texel.y * tWidth) * 4;

							if (triangleTexture[index2 + 3] > 0)
							{
								buffer[index] = triangleTexture[index2];
								buffer[index + 1] = triangleTexture[index2 + 1];
								buffer[index + 2] = triangleTexture[index2 + 2];
								buffer[index + 3] = triangleTexture[index2 + 3];
								z_buffer[j][i] = inv_z;
							}

							
						}
					}
				}

			}

		}
	}
}



int main()
{
	Rasterizer rasterizer = Rasterizer(1000, 1000, .1, 10, 60);
	sf::RenderWindow window(sf::VideoMode(rasterizer.w_width, rasterizer.w_height), "3D Render");

	sf::Clock clock = sf::Clock::Clock();
	sf::Time last_time = clock.getElapsedTime();
	sf::Time cur_time;
	sf::Font font;
	float fps;
	font.loadFromFile("arial.ttf");

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
	Camera cam;
	cam.setPanSpeed(5);
	cam.setRotationSpeed(1);

	int bufferSize = rasterizer.w_width * rasterizer.w_height * 4;

	
	int ft = 0;
	while (window.isOpen())
	{
		//DECLARE/RESET our buffers
		sf::Uint8* buffer = new sf::Uint8[(1000 * 1000 * 4)];
		std::vector<std::vector<float>> z_buffer((float)rasterizer.w_width, std::vector<float>(rasterizer.w_height, INT_MAX));
		for (int i = 0; i < bufferSize; i++)
		{
			buffer[i] = 0;
		}

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
			camera_pan_forward, camera_pan_backwards, camera_pan_left, camera_pan_right);

		std::vector<std::vector<Triangle>> triangleLists(11);
		for (int i = 0; i < 10; ++i)
		{
			triangleLists[i] = std::vector<Triangle>();
		}

		int list = 0;
		for (auto& triangle : scene.sceneData)
		{
			sf::Vector3f view_verts[3];

			func::vecXmatrix(triangle.verts[0], cam.camMatrix, view_verts[0]);
			func::vecXmatrix(triangle.verts[1], cam.camMatrix, view_verts[1]);
			func::vecXmatrix(triangle.verts[2], cam.camMatrix, view_verts[2]);


			rasterizer.clip_triangle_near(triangle, view_verts, triangleLists[list]);
			list++;
			if (list > 10)
				list = 0;
		}
		

		//foo(triangleLists[0], rasterizer, z_buffer, scene, buffer);
		
		g.run([&] {foo(triangleLists[0], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[1], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[2], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[3], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[4], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[5], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[6], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[7], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[8], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[9], rasterizer, z_buffer, scene, buffer); });
		g.run([&] {foo(triangleLists[10], rasterizer, z_buffer, scene, buffer); });
		g.wait();
		
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


		//float h1 = scene.heightMap.find(names[ft])->second;
		//float w1 = scene.widthMap.find(names[ft])->second;
		//image.create(w1, h1, d);
		image.create(rasterizer.w_width, rasterizer.w_height, buffer);
		delete[] buffer;

		sf::Texture texture;
		texture.loadFromImage(image);

		sf::Sprite sprite;
		sprite.setTexture(texture);

		window.draw(sprite);
		window.draw(frames);
		window.draw(numTrisText);

		window.display();
		//system("Pause");
		ft++;
	}
	return 0;
}