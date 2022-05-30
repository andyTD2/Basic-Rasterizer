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

void foo(std::vector<Triangle>& preparedTriangles, Rasterizer& rasterizer, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer, sf::Uint8*& datar)
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
							z_buffer[j][i] = inv_z;

							
							int index = (j + i * rasterizer.w_width) * 4;
							/*
							texel.x = b0t * triangle.tCoords[0].x +
								b1t * triangle.tCoords[1].x +
								b2t * triangle.tCoords[2].x;

							texel.y = b0t * triangle.tCoords[0].y +
								b1t * triangle.tCoords[1].y +
								b2t * triangle.tCoords[2].y;

							texel.x *= 1024;
							texel.y *= 1024;
							

							int index2 = ((int)texel.x + (int)texel.y * 1024) * 4;
							buffer[index] = datar[index2];
							buffer[index + 1] = datar[index2 + 1];
							buffer[index + 2] = datar[index2 + 2];
							buffer[index + 3] = datar[index2 + 3];
							*/

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

							texel.x *= 1023;
							texel.y *= 1023;
							//std::cout << texel.x << " " << texel.y << std::endl;
							int index2 = ((int)texel.x + (int)texel.y * 1024) * 4;
							//std::cout << index2 << std::endl;

							buffer[index] = datar[index2];
							buffer[index + 1] = datar[index2 + 1];
							buffer[index + 2] = datar[index2 + 2];
							buffer[index + 3] = datar[index2 + 3];


								
						}
					}
				}

			}

		}
	}
}



int main()
{
	Rasterizer rasterizer = Rasterizer(1024, 1024, .1, 10, 60);
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

	
	unsigned char header_[12];
	unsigned char bpp_;
	unsigned char id_;
	unsigned short width_;
	unsigned short height_;
	//unsigned char* data_;

	std::ifstream file;

	std::vector<std::string> fileNames =
	{ "lion.tga", "background.tga", "vase_plant.tga", "sponza_arch_diff.tga",
	"spnza_bricks_a_diff.tga", "sponza_ceiling_a_diff.tga", "chain_texture.tga",
	"sponza_column_a_diff.tga", "sponza_column_b_diff.tga", "sponza_column_c_diff.tga",
	"sponza_details_diff.tga", "sponza_fabric_diff.tga", "sponza_curtain_diff.tga",
	"sponza_fabric_blue_diff.tga", "sponza_fabric_green_diff.tga", "sponza_curtain_green_diff.tga",
	"sponza_curtain_blue_diff.tga", "sponza_flagpole_diff.tga", "sponza_floor_a_diff.tga",
	"sponza_thorn_diff.tga", "sponza_roof_diff.tga", "vase_dif.tga", "vase_hanging.tga",
	"vase_round.tga" };
	
	file.open("lion.tga", std::ios::binary);

	if (file.fail())
	{
		std::cout << "lion.tga" << " could not be opened." << std::endl;
		return 0;
	}

	//file.seekg(0);
	file.read((char*)header_, sizeof(unsigned char) * 12);
	file.read((char*)&width_, sizeof(unsigned short));
	file.read((char*)&height_, sizeof(unsigned short));
	file.read((char*)&bpp_, sizeof(unsigned char));
	file.read((char*)&id_, sizeof(unsigned char));

	for (int i = 0; i < 8; ++i)
	{
		char temp = id_;
		std::cout << ((temp & 1 << i)) << std::endl;
	}


	int tSize = (int)width_ * (int)height_;
	sf::Uint8* data = new sf::Uint8[tSize * 4];
	sf::Uint8* datar = new sf::Uint8[tSize * 4];
	file.read((char*)data, tSize * 4);


	int k = 0;
	int rowIncr = height_ * 4;
	int colIncr = width_ * 4;

	/*
	int curRow = tSize * 4 - rowIncr;
	while (curRow >= 0)
	{
		for (int i = curRow; i < (curRow + colIncr); i+=4)
		{
			datar[k] = data[i + 2];
			datar[k + 1] = data[i + 1];
			datar[k + 2] = data[i];
			datar[k + 3] = data[i + 3];
			k += 4;
		}
		curRow -= rowIncr;
	}*/
	
	int curRow = 0;
	while (curRow < tSize * 4 - rowIncr)
	{
		for (int i = curRow; i < (curRow + colIncr); i+=4)
		{
			datar[k] = data[i + 2];
			datar[k + 1] = data[i + 1];
			datar[k + 2] = data[i];
			datar[k + 3] = data[i + 3];
			k += 4;
		}
		curRow += rowIncr;
	}

	file.close();
	
	//scene.sceneData.clear();
	//scene.sceneData.push_back(Triangle(sf::Vector3f(-.5, .5, 1), sf::Vector3f(.5, .5, 1), sf::Vector3f(.5, -.5, 1), sf::Vector2f(0, 0), sf::Vector2f(1, 0), sf::Vector2f(1, 1)));
	//scene.sceneData.push_back(Triangle(sf::Vector3f(-.5, -.5, 1), sf::Vector3f(-.5, .5, 1), sf::Vector3f(.5, -.5, 1), sf::Vector2f(0, 1), sf::Vector2f(0, 0), sf::Vector2f(1, 1)));
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


			rasterizer.clip_triangle_near(triangle, view_verts, triangleLists[list]);
			list++;
			if (list > 10)
				list = 0;
		}
		

		g.run([&] {foo(triangleLists[0], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[1], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[2], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[3], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[4], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[5], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[6], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[7], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[8], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[9], rasterizer, z_buffer, buffer, datar); });
		g.run([&] {foo(triangleLists[10], rasterizer, z_buffer, buffer, datar); });
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

		/*
		int i = 0;
		int j = 4104;
		while (j < tSize * 4)
		{
			buffer[i] = datar[j];
			buffer[i + 1] = datar[j + 1];
			buffer[i + 2] = datar[j +2];
			buffer[i + 3] = datar[j + 3];
			j += 4;
			i += 4;
		}
		for (int i = 0; i < bufferSize; ++i)
		{
			if ((int)buffer[i] != 0)
			std::cout << (int)buffer[i] << std::endl;
		}
		*/

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