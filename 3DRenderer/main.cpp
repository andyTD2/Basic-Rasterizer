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
#include <time.h>

struct Tile
{
	int bLeft;
	int bTop;
	int bBot;
	int bRight;

	float maxZ = FLT_MAX;
	std::vector<Triangle*> trianglesToRender;

};

void foobar(std::vector<Triangle*>& list, Rasterizer& rasterizer)
{
	for (auto& tri : list)
	{
		rasterizer.project_triangle(*tri, rasterizer.p_mat);
	}
}
void toViewSpace(const Rasterizer& rasterizer, std::vector<Triangle*>& out, const Camera& cam, Triangle& triangle, std::vector<Triangle*>& trisToDelete)
{
//
	//for (int i = start; i < end; ++i)
	//{
		//Triangle triangle = sceneData[i];

		bool culled = false;
		if (func::getDist(cam.rightNormal, cam.camPos, triangle.verts[0]) < 0 &&
			func::getDist(cam.rightNormal, cam.camPos, triangle.verts[1]) < 0 &&
			func::getDist(cam.rightNormal, cam.camPos, triangle.verts[2]) < 0)
			culled = true;
		if (func::getDist(cam.leftNormal, cam.camPos, triangle.verts[0]) < 0 &&
			func::getDist(cam.leftNormal, cam.camPos, triangle.verts[1]) < 0 &&
			func::getDist(cam.leftNormal, cam.camPos, triangle.verts[2]) < 0)
			culled = true;
		if (func::getDist(cam.topNormal, cam.camPos, triangle.verts[0]) < 0 &&
			func::getDist(cam.topNormal, cam.camPos, triangle.verts[1]) < 0 &&
			func::getDist(cam.topNormal, cam.camPos, triangle.verts[2]) < 0)
			culled = true;
		if (func::getDist(cam.botNormal, cam.camPos, triangle.verts[0]) < 0 &&
			func::getDist(cam.botNormal, cam.camPos, triangle.verts[1]) < 0 &&
			func::getDist(cam.botNormal, cam.camPos, triangle.verts[2]) < 0)
			culled = true;


		//if (bLeft < rasterizer.w_width && bRight > -rasterizer.w_width && bTop < rasterizer.w_height && bBot > -rasterizer.w_height)
		if (!culled)
		{
			func::vecXmatrix(triangle.verts[0], cam.camMatrix, triangle.transVerts[0], false);
			func::vecXmatrix(triangle.verts[1], cam.camMatrix, triangle.transVerts[1], false);
			func::vecXmatrix(triangle.verts[2], cam.camMatrix, triangle.transVerts[2], false);
			rasterizer.clip_triangle_near(triangle, out, trisToDelete);
		}
	//}

}
void bar(Tile& tile, Scene& scene, std::vector<std::vector<float>>& z_buffer, Rasterizer& rasterizer, sf::Uint8*& buffer)
{
	for (auto& triangle : tile.trianglesToRender)
	{
		int bLeft = std::max(tile.bLeft, std::min(triangle->bLeft, tile.bRight));
		int bTop = std::max(tile.bTop, std::min(triangle->bTop, tile.bBot));
		int bRight = std::min(tile.bRight, std::max(triangle->bRight, tile.bLeft));
		int bBot = std::min(tile.bBot, std::max(triangle->bBot, tile.bTop));

		float a0 = (triangle->projVerts[2].y - triangle->projVerts[1].y), b0 = (triangle->projVerts[1].x - triangle->projVerts[2].x);
		float a1 = (triangle->projVerts[0].y - triangle->projVerts[2].y), b1 = (triangle->projVerts[2].x - triangle->projVerts[0].x);
		float a2 = (triangle->projVerts[1].y - triangle->projVerts[0].y), b2 = (triangle->projVerts[0].x - triangle->projVerts[1].x);

		__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
		__m256 A0s = _mm256_mul_ps(_mm256_set_ps(a0, a0, a0, a0, a0, a0, a0, a0), factor);
		__m256 A1s = _mm256_mul_ps(_mm256_set_ps(a1, a1, a1, a1, a1, a1, a1, a1), factor);
		__m256 A2s = _mm256_mul_ps(_mm256_set_ps(a2, a2, a2, a2, a2, a2, a2, a2), factor);
		float incA0 = a0 * 8;
		float incA1 = a1 * 8;
		float incA2 = a2 * 8;

		vec2 pixel(std::max(bLeft, 0), std::max(0, bTop));
		float w0r = func::edge_f(pixel, triangle->projVerts[1], triangle->projVerts[2]);
		float w1r = func::edge_f(pixel, triangle->projVerts[2], triangle->projVerts[0]);
		float w2r = func::edge_f(pixel, triangle->projVerts[0], triangle->projVerts[1]);

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
						float b0t = one[k] / (triangle->area);
						float b1t = two[k] / (triangle->area);
						float b2t = three[k] / (triangle->area);

						float z = b0t * triangle->vertexDepth[0] +
							b1t * triangle->vertexDepth[1] +
							b2t * triangle->vertexDepth[2];

						vec2 texel;

						float inv_z = 1 / z;
						int zindex = k + j;
						if (inv_z < z_buffer[zindex][i])
						{
							int index = (zindex + i * rasterizer.w_width) * 4;

							float u = (triangle->tx[0] * b0t) +
								(triangle->tx[1] * b1t) +
								(triangle->tx[2] * b2t);

							float v = (triangle->ty[0] * b0t) +
								(triangle->ty[1] * b1t) +
								(triangle->ty[2] * b2t);


							texel.x = u / z;
							texel.y = v / z;


							if (texel.x < 0 || texel.x > 0)
								texel.x = texel.x - floor(texel.x);
							if (texel.y < 0 || texel.y > 0)
								texel.y = texel.y - floor(texel.y);

							texel.x *= (triangle->tWidth - 1);
							texel.y *= (triangle->tHeight - 1);



							int index2 = (floor(texel.x) + floor(texel.y) * triangle->tWidth) * 4;
							if (triangle->triangleTexture[index2 + 3] > 0)
							{
								z_buffer[zindex][i] = inv_z;
								buffer[index] = triangle->triangleTexture[index2];
								buffer[index + 1] = triangle->triangleTexture[index2 + 1];
								buffer[index + 2] = triangle->triangleTexture[index2 + 2];
								buffer[index + 3] = triangle->triangleTexture[index2 + 3];
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
	Camera cam(1, 5);

	int bufferSize = rasterizer.w_width * rasterizer.w_height * 4;


	int ft = 0;
	int numTiles = 8;

	while (window.isOpen())
	{
		std::cout << scene.sceneData.size() << std::endl;
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

		std::vector<std::vector<Triangle*>> triangleLists(11, std::vector<Triangle*>());
		std::vector<std::vector<Triangle*>> trisToDelete(11, std::vector<Triangle*>());


		std::clock_t t;
		t = std::clock();
		{
			int increment = scene.sceneData.size() / 11;
			int start = 0;
			int end = start + increment;
			int i = 0;
			while (i < 10)
			{
				//toViewSpace(start, end, scene.sceneData, cam.camMatrix, rasterizer, triangleLists[i]);
				g.run([&, start, end, i] 
				{
					for (int j = start; j < end; ++j) 
					{
						if (!cam.checkIfTriangleCulled(scene.sceneData[j]))
						{
							cam.transformToViewSpace(scene.sceneData[j]);
							rasterizer.clip_triangle_near(scene.sceneData[j], triangleLists[i], trisToDelete[i]);
						}
						//toViewSpace(rasterizer, triangleLists[i], cam, scene.sceneData[j], trisToDelete[i]);
					}
				});
				start = end;
				end = start + increment;
				i++;
			}
			//g.run([&, start, i] {toViewSpace(start, scene.sceneData.size(), scene.sceneData, cam.camMatrix, rasterizer, triangleLists[i], cam); });
			g.run([&, start, i]
			{
				for (int j = start; j < scene.sceneData.size(); ++j)
				{
					if (!cam.checkIfTriangleCulled(scene.sceneData[j]))
					{
						cam.transformToViewSpace(scene.sceneData[j]);
						rasterizer.clip_triangle_near(scene.sceneData[j], triangleLists[i], trisToDelete[i]);
					}
				}
			});
			g.wait();
		}
		t = std::clock() - t;
		std::cout << "viewspace and clipping: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;

		int tileLengthHorizontal = rasterizer.w_width / numTiles;
		int tileLengthVertical = rasterizer.w_height / numTiles;

		std::vector<std::vector<Tile>> tiles(numTiles, std::vector<Tile>(numTiles));

		t = std::clock();
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
		t = std::clock() - t;
		std::cout << "tile setup: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;

		t = std::clock();
		//std::vector<Triangle> rasterList;
		for (int i = 0; i < triangleLists.size(); ++i)
		{
			g.run([&, i] {foobar(triangleLists[i], rasterizer); });
			//foobar(triangleLists[i], rasterizer, rasterLists[i]);
		}
		g.wait();

	//	for (auto& list : triangleLists)
	//		for (auto& ptr : list)
	//			delete ptr;

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
		for (auto& list : triangleLists)
		{
			for (auto& tri : list)
			{

				int startCol = tri->bLeft / tileLengthHorizontal;
				int endCol = tri->bRight / tileLengthHorizontal;
				int startRow = tri->bTop / tileLengthVertical;
				int endRow = tri->bBot / tileLengthVertical;
				
				//std::cout << "startCol: " << startCol << ", endCol: " << endCol << std::endl;
				//std::cout << "startRow: " << startRow << ", endRow: " << endRow << std::endl;
				for (int i = startCol; i <= endCol; i++)
				{
					for (int j = startRow; j <= endRow; ++j)
					{
						//std::cout << i << ", " << j << std::endl;
						tiles[i][j].trianglesToRender.push_back(tri);
					}
				}
			}
		}

		t = std::clock() - t;
		std::cout << "binning: " << (float)t / CLOCKS_PER_SEC << " seconds" << std::endl;

		t = std::clock();
		for (auto& row : tiles)
		{
			for (auto& tile : row)
				g.run([&] {bar(tile, scene, z_buffer, rasterizer, buffer); });
				//bar(tile, scene, z_buffer, rasterizer, buffer);
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