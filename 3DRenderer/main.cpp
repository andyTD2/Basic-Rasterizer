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

std::mutex m;

void foo(std::vector<Triangle>& preparedTriangles, Rasterizer& rasterizer, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer)
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

		if (bLeft < rasterizer.w_width && bRight > -1 && bTop < rasterizer.w_height - 1 && bBot > -1)
		{
			bLeft = std::max(0, std::min(bLeft, rasterizer.w_width - 1));
			bTop = std::max(0, std::min(bTop, rasterizer.w_height - 1));
			bRight = std::min(rasterizer.w_width - 1, std::max(bRight, 0));
			bBot = std::min(rasterizer.w_height - 1, std::max(bBot, 0));

			float area = func::edge_f(sf::Vector2f(proj_verts[0].x, proj_verts[0].y), proj_verts[1], proj_verts[2]);

			float a0 = (proj_verts[2].y - proj_verts[1].y), b0 = (proj_verts[1].x - proj_verts[2].x);
			float a1 = (proj_verts[0].y - proj_verts[2].y), b1 = (proj_verts[2].x - proj_verts[0].x);
			float a2 = (proj_verts[1].y - proj_verts[0].y), b2 = (proj_verts[0].x - proj_verts[1].x);


			__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
			__m256 A0s = _mm256_mul_ps(_mm256_set_ps(a0, a0, a0, a0, a0, a0, a0, a0), factor);
			__m256 A1s = _mm256_mul_ps(_mm256_set_ps(a1, a1, a1, a1, a1, a1, a1, a1), factor);
			__m256 A2s = _mm256_mul_ps(_mm256_set_ps(a2, a2, a2, a2, a2, a2, a2, a2), factor);
			float incA0 = a0 * 8;
			float incA1 = a1 * 8;
			float incA2 = a2 * 8;

			sf::Vector2f pixel(std::max(bLeft, 0) + .5, std::max(0, bTop) + .5);
			float w0r = func::edge_f(pixel, proj_verts[1], proj_verts[2]);
			float w1r = func::edge_f(pixel, proj_verts[2], proj_verts[0]);
			float w2r = func::edge_f(pixel, proj_verts[0], proj_verts[1]);



			for (int i = bTop; i < bBot; i += 1)
			{
				__m256 w0 = _mm256_add_ps(A0s, _mm256_set_ps(w0r, w0r, w0r, w0r, w0r, w0r, w0r, w0r));
				__m256 w1 = _mm256_add_ps(A1s, _mm256_set_ps(w1r, w1r, w1r, w1r, w1r, w1r, w1r, w1r));
				__m256 w2 = _mm256_add_ps(A2s, _mm256_set_ps(w2r, w2r, w2r, w2r, w2r, w2r, w2r, w2r));

				float* one = (float*)&w0;
				float* two = (float*)&w1;
				float* three = (float*)&w2;
				for (int j = bLeft; j < bRight; j += 8)
				{
					for (int k = 0; k < 8 && k + j < bRight; ++k)
					{
						//USE THE FIRST IF STATEMENT IF YOU DONT WANT BACKFACE CULLING
						//if ((one[k] >= 0 && two[k] >= 0 && three[k] >= 0) || ((one[k] < 0 && two[k] < 0 && three[k] < 0)))
						if (one[k] < 0 && two[k] < 0 && three[k] < 0)
						{
							float b0t = one[k] / (area);
							float b1t = two[k] / (area);
							float b2t = three[k] / (area);

							float z = b0t * va +
								b1t * vb +
								b2t * vc;

							float inv_z = 1 / z;
							int index = k + j;

							if (inv_z < z_buffer[index][i])
							{
								z_buffer[index][i] = inv_z;
								index = (index + i * 1000) * 4;
								buffer[index] = b0t * 255;
								buffer[index + 1] = b1t * 255;
								buffer[index + 2] = b2t * 255;
								buffer[index + 3] = 255;
							}
						}
					}
					w0 = _mm256_add_ps(w0, _mm256_set_ps(incA0, incA0, incA0, incA0, incA0, incA0, incA0, incA0));
					w1 = _mm256_add_ps(w1, _mm256_set_ps(incA1, incA1, incA1, incA1, incA1, incA1, incA1, incA1));
					w2 = _mm256_add_ps(w2, _mm256_set_ps(incA2, incA2, incA2, incA2, incA2, incA2, incA2, incA2));
				}
				w0r += b0;
				w1r += b1;
				w2r += b2;
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

	bool camera_rotating_right = false, camera_rotating_left = false, camera_rotating_up = false, camera_rotating_down = false;
	bool camera_pan_forward = false, camera_pan_backwards = false, camera_pan_left = false, camera_pan_right = false;
	Camera cam;
	cam.setPanSpeed(5);
	cam.setRotationSpeed(1);

	int bufferSize = rasterizer.w_width * rasterizer.w_height * 4;

	while (window.isOpen())
	{
		//DECLARE/RESET our buffers
		sf::Uint8* buffer = new sf::Uint8[bufferSize];
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

			rasterizer.clip_triangle_near(view_verts, triangleLists[list]);

			list++;
			if (list > 10)
				list = 0;
		}

		g.run([&] {foo(triangleLists[0], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[1], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[2], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[3], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[4], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[5], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[6], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[7], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[8], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[9], rasterizer, z_buffer, buffer); });
		g.run([&] {foo(triangleLists[10], rasterizer, z_buffer, buffer); });
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
	}
	return 0;
}