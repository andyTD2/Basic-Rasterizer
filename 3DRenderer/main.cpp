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

struct Tile
{
	int bLeft;
	int bTop;
	int bBot;
	int bRight;

	float maxZ = FLT_MAX;
	std::vector<Triangle> trianglesToRender;
};


void toViewSpace(int start, int end, const std::vector<Triangle>& sceneData, const float camMatrix[4][4], const Rasterizer& rasterizer, std::vector<Triangle>& out)
{

	for (int i = start; i < end; ++i)
	{
		Triangle triangle = sceneData[i];
		sf::Vector3f view_verts[3];

		Triangle outT;
		outT.associatedMtl = triangle.associatedMtl;
		outT.tCoords[0] = triangle.tCoords[0];
		outT.tCoords[1] = triangle.tCoords[1];
		outT.tCoords[2] = triangle.tCoords[2];
		//func::vecXmatrix(triangle.verts[0], camMatrix, outT.verts[0]);
		//func::vecXmatrix(triangle.verts[1], camMatrix, outT.verts[1]);
		//func::vecXmatrix(triangle.verts[2], camMatrix, outT.verts[2]);
		func::vecXmatrix(triangle.verts[0], camMatrix, view_verts[0]);
		func::vecXmatrix(triangle.verts[1], camMatrix, view_verts[1]);
		func::vecXmatrix(triangle.verts[2], camMatrix, view_verts[2]);

		////float bLeft = std::min({ view_verts[0].x, view_verts[1].x, view_verts[2].x });
		//float bTop = std::min({ view_verts[0].y, view_verts[1].y, view_verts[2].y });
		//float bRight = std::max({ view_verts[0].x, view_verts[1].x, view_verts[2].x });
		//float bBot = std::max({ view_verts[0].y, view_verts[1].y, view_verts[2].y });

		//if (bLeft < rasterizer.w_width && bRight > -rasterizer.w_width && bTop < rasterizer.w_height && bBot > -rasterizer.w_height)
		rasterizer.clip_triangle_near(triangle, view_verts, out);
		//out.push_back(triangle);
	}

}
void bar(Tile& tile, Scene& scene, std::vector<std::vector<float>>& z_buffer, Rasterizer& rasterizer, sf::Uint8*& buffer)
{
	for (auto& triangle : tile.trianglesToRender)
	{
		float va = 1 / triangle.verts[0].z;
		float vb = 1 / triangle.verts[1].z;
		float vc = 1 / triangle.verts[2].z;

		sf::Uint8* triangleTexture = scene.textureData.find(triangle.associatedMtl)->second;
		int tWidth = scene.widthMap.find(triangle.associatedMtl)->second;
		int tHeight = scene.heightMap.find(triangle.associatedMtl)->second;

		float* tx = new float[3];
		float* ty = new float[3];
		tx[0] = triangle.tCoords[0].x / triangle.verts[0].z; tx[1] = triangle.tCoords[1].x / triangle.verts[1].z; tx[2] = triangle.tCoords[2].x / triangle.verts[2].z;
		ty[0] = triangle.tCoords[0].y / triangle.verts[0].z; ty[1] = triangle.tCoords[1].y / triangle.verts[1].z; ty[2] = triangle.tCoords[2].y / triangle.verts[2].z;


		int bLeft = std::max(tile.bLeft, std::min(triangle.bLeft, tile.bRight));
		int bTop = std::max(tile.bTop, std::min(triangle.bTop, tile.bBot));
		int bRight = std::min(tile.bRight, std::max(triangle.bRight, tile.bLeft));
		int bBot = std::min(tile.bBot, std::max(triangle.bBot, tile.bTop));

		float area = func::edge_f(sf::Vector2f(triangle.projVerts[0].x, triangle.projVerts[0].y), triangle.projVerts[1], triangle.projVerts[2]);

		float a0 = (triangle.projVerts[2].y - triangle.projVerts[1].y), b0 = (triangle.projVerts[1].x - triangle.projVerts[2].x);
		float a1 = (triangle.projVerts[0].y - triangle.projVerts[2].y), b1 = (triangle.projVerts[2].x - triangle.projVerts[0].x);
		float a2 = (triangle.projVerts[1].y - triangle.projVerts[0].y), b2 = (triangle.projVerts[0].x - triangle.projVerts[1].x);

		__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
		__m256 A0s = _mm256_mul_ps(_mm256_set_ps(a0, a0, a0, a0, a0, a0, a0, a0), factor);
		__m256 A1s = _mm256_mul_ps(_mm256_set_ps(a1, a1, a1, a1, a1, a1, a1, a1), factor);
		__m256 A2s = _mm256_mul_ps(_mm256_set_ps(a2, a2, a2, a2, a2, a2, a2, a2), factor);
		float incA0 = a0 * 8;
		float incA1 = a1 * 8;
		float incA2 = a2 * 8;

		sf::Vector2f pixel(std::max(bLeft, 0) + .5, std::max(0, bTop) + .5);
		float w0r = func::edge_f(pixel, triangle.projVerts[1], triangle.projVerts[2]);
		float w1r = func::edge_f(pixel, triangle.projVerts[2], triangle.projVerts[0]);
		float w2r = func::edge_f(pixel, triangle.projVerts[0], triangle.projVerts[1]);

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
					if ((one[k] <= 0 && two[k] <= 0 && three[k] <= 0) || (one[k] >= 0 && two[k] >= 0 && three[k] >= 0))
						//if ((one[k] <= 0 && two[k] <= 0 && three[k] <= 0))
					{
						float b0t = one[k] / (area);
						float b1t = two[k] / (area);
						float b2t = three[k] / (area);

						float z = b0t * va +
							b1t * vb +
							b2t * vc;

						sf::Vector2f texel;

						float inv_z = 1 / z;
						int zindex = k + j;
						if (inv_z < z_buffer[zindex][i])
						{
							int index = (zindex + i * rasterizer.w_width) * 4;

							float w = (va * b0t) +
								(vb * b1t) +
								(vc * b2t);

							float u = (tx[0] * b0t) +
								(tx[1] * b1t) +
								(tx[2] * b2t);

							float v = (ty[0] * b0t) +
								(ty[1] * b1t) +
								(ty[2] * b2t);


							texel.x = u / w;
							texel.y = v / w;


							if (texel.x < 0 || texel.x > 0)
								texel.x = texel.x - floor(texel.x);
							if (texel.y < 0 || texel.y > 0)
								texel.y = texel.y - floor(texel.y);

							texel.x *= (tWidth - 1);
							texel.y *= (tHeight - 1);



							int index2 = (floor(texel.x) + floor(texel.y) * tWidth) * 4;
							if (triangleTexture[index2 + 3] > 0)
							{
								z_buffer[zindex][i] = inv_z;
								buffer[index] = triangleTexture[index2];
								buffer[index + 1] = triangleTexture[index2 + 1];
								buffer[index + 2] = triangleTexture[index2 + 2];
								buffer[index + 3] = triangleTexture[index2 + 3];
							}


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

		delete[]tx;
		delete[]ty;
	}
}

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
	int numTiles = 8;

	while (window.isOpen())
	{
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
			camera_pan_forward, camera_pan_backwards, camera_pan_left, camera_pan_right);

		std::vector<std::vector<Triangle>> triangleLists(11, std::vector<Triangle>());



		{
			int increment = scene.sceneData.size() / 11;
			int start = 0;
			int end = start + increment;
			int i = 0;
			while (i < 10)
			{
				//toViewSpace(start, end, scene.sceneData, cam.camMatrix, rasterizer, triangleLists[i]);
				g.run([&, start, end, i] {toViewSpace(start, end, scene.sceneData, cam.camMatrix, rasterizer, triangleLists[i]); });
				start = end;
				end = start + increment;
				i++;
			}
			//g.run([&, start, i] {toViewSpace(start, scene.sceneData.size(), scene.sceneData, cam.camMatrix, rasterizer, triangleLists[i]); });
			g.wait();
		}




		int tileLengthHorizontal = rasterizer.w_width / numTiles;
		int tileLengthVertical = rasterizer.w_height / numTiles;

		std::vector<std::vector<Tile>> tiles(numTiles, std::vector<Tile>(numTiles));


		for (int i = 0; i < numTiles; ++i)
		{
			for (int j = 0; j < numTiles; ++j)
			{
				Tile tile;
				tile.bLeft = i * tileLengthHorizontal;
				tile.bRight = tile.bLeft + tileLengthHorizontal;
				tile.bTop = j * tileLengthVertical;
				tile.bBot = tile.bTop + tileLengthVertical;
				tiles[i][j] = tile;
			}
		}



		for (auto& list : triangleLists)
		{
			for (auto& tri : list)
			{
				if (rasterizer.project_triangle(tri, rasterizer.p_mat))
				{

					tri.bLeft = std::min({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
					tri.bTop = std::min({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });
					tri.bRight = std::max({ tri.projVerts[0].x, tri.projVerts[1].x, tri.projVerts[2].x });
					tri.bBot = std::max({ tri.projVerts[0].y, tri.projVerts[1].y, tri.projVerts[2].y });

					for (auto& col : tiles)
					{
						for (auto& tile : col)
						{
							if (tri.bLeft < tile.bRight && tri.bRight > tile.bLeft && tri.bTop < tile.bBot && tri.bBot > tile.bTop)
							{
								tile.trianglesToRender.push_back(tri);
							}
						}
					}
				}
			}
		}

		for (auto& row : tiles)
		{
			for (auto& tile : row)
				g.run([&] {bar(tile, scene, z_buffer, rasterizer, buffer); });
		}

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
		ft++;
	}
	return 0;
}