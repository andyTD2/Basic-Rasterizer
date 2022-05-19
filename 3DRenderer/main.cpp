#include <SFML/Graphics.hpp>
#include <iostream>
#include <vector>
#include <math.h>
#include <iomanip>
#include "Rasterizer.hpp"
#include <stdlib.h>
#include "immintrin.h"


//c41


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

	int size = rasterizer.w_width * rasterizer.w_height * 4;


	//top face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 2, 3), sf::Vector3f(2, 2, 4), sf::Vector3f(2, 2, 3)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 2, 3), sf::Vector3f(1, 2, 4), sf::Vector3f(2, 2, 4)));

	//right face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(2, 1, 4), sf::Vector3f(2, 2, 3), sf::Vector3f(2, 1, 3)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(2, 1, 4), sf::Vector3f(2, 2, 3), sf::Vector3f(2, 2, 4)));

	//back face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 4), sf::Vector3f(2, 1, 4), sf::Vector3f(2, 2, 4)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 4), sf::Vector3f(2, 2, 4), sf::Vector3f(1, 2, 4)));


	//front face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 3), sf::Vector3f(2, 1, 3), sf::Vector3f(2, 2, 3)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 3), sf::Vector3f(2, 2, 3), sf::Vector3f(1, 2, 3)));

	//left face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 3), sf::Vector3f(1, 2, 3), sf::Vector3f(1, 1, 4)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 4), sf::Vector3f(1, 2, 3), sf::Vector3f(1, 2, 4)));

	//bot face
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 3), sf::Vector3f(2, 1, 4), sf::Vector3f(2, 1, 3)));
	rasterizer.triangles.push_back(Triangle(sf::Vector3f(1, 1, 3), sf::Vector3f(1, 1, 4), sf::Vector3f(2, 1, 4)));




	//right -> left -> mid
	//rasterizer.triangles.push_back(Triangle(sf::Vector3f(.25, -.1, 1), sf::Vector3f(-.25, -.1, 1), sf::Vector3f(0, -.1, 1.5)));
	//rasterizer.triangles.push_back(Triangle(sf::Vector3f(.25, .1, 1.75), sf::Vector3f(-.25, .1, 1.75), sf::Vector3f(0, .1, .5)));
	

	float c2w[4][4] = { 0 };
	float inv[4][4] = { 0 };

	bool camera_rotating_right = false, camera_rotating_left = false, camera_rotating_up = false, camera_rotating_down = false;
	bool camera_pan_forward = false, camera_pan_backward = false, camera_pan_left = false, camera_pan_right = false;
	float theta = 0, camera_pan = 0, camera_x_rotation = 0, camera_y_rotation = 0;

	bool debug = false;
	sf::Vector3f camera(1.5, 1.5, 0);
	//sf::Vector3f cam_tar(0, 0, 1);
	sf::Vector3f vLookDir(0,0,0);
	while (window.isOpen())
	{
		sf::Uint8* buffer = new sf::Uint8[size];
		for (int i = 0; i < size; i++)
		{
			buffer[i] = 0;
		}


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
				if (event.key.code == sf::Keyboard::S) camera_pan_backward = true;
				if (event.key.code == sf::Keyboard::A) camera_pan_left = true;
				if (event.key.code == sf::Keyboard::D) camera_pan_right = true;
				if (event.key.code == sf::Keyboard::F) debug = true;
				if (event.key.code == sf::Keyboard::G) debug = false;
			}
			if (event.type == sf::Event::KeyReleased)
			{
				if (event.key.code == sf::Keyboard::Right) camera_rotating_right = false;
				if (event.key.code == sf::Keyboard::Left) camera_rotating_left = false;
				if (event.key.code == sf::Keyboard::Up) camera_rotating_up = false;
				if (event.key.code == sf::Keyboard::Down) camera_rotating_down = false;
				if (event.key.code == sf::Keyboard::W) camera_pan_forward = false;
				if (event.key.code == sf::Keyboard::S) camera_pan_backward = false;
				if (event.key.code == sf::Keyboard::A) camera_pan_left = false;
				if (event.key.code == sf::Keyboard::D) camera_pan_right = false;
			}

		}
		sf::Vector3f vForward;
		vForward.x = vLookDir.x * .1;
		vForward.y = vLookDir.y * .1;
		vForward.z = vLookDir.z * .1;
		if (camera_rotating_right) camera_x_rotation += .5;
		if (camera_rotating_left) camera_x_rotation -= .5;
		if (camera_rotating_down) camera_y_rotation += .5;
		if (camera_rotating_up) camera_y_rotation -= .5;
		if (camera_pan_forward) camera+= vForward;
		if (camera_pan_backward) camera -= vForward;
		if (camera_pan_left)
		{
			sf::Vector3f t0 = func::norm3f(func::crossv(vForward, sf::Vector3f(0, 1, 0)));
			t0.x *= .1;
			t0.y *= .1;
			t0.z *= .1;
			
			camera += t0;
		}
		if (camera_pan_right)
		{
			sf::Vector3f t1 = func::norm3f(func::crossv(vForward, sf::Vector3f(0, 1, 0)));
			t1.x *= .1;
			t1.y *= .1;
			t1.z *= .1;

			camera -= t1;
		}

		if (camera_y_rotation > 89) camera_y_rotation = 89;
		else if (camera_y_rotation < -89) camera_y_rotation = -89;

		//prevent overflow 
		if (camera_x_rotation < -360) camera_x_rotation += 360;
		else if (camera_x_rotation > 360) camera_x_rotation -= 360;


		//RENDER
		window.clear();

		float y_mat[4][4] = {
		{cos(camera_x_rotation * 3.14159265359 / 180), 0.0, -sin(camera_x_rotation * 3.14159265359 / 180), 0.0},
		{0.0, 1.0, 0.0, 0.0 },
		{sin(camera_x_rotation * 3.14159265359 / 180), 0.0, cos(camera_x_rotation * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
		};

		float x_mat[4][4] = {
		{1.0, 0.0, 0.0, 0.0},
		{0.0, cos(camera_y_rotation * 3.14159265359 / 180), sin(camera_y_rotation * 3.14159265359 / 180), 0.0},
		{0.0, -sin(camera_y_rotation * 3.14159265359 / 180), cos(camera_y_rotation * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
		};




		sf::Vector3f vTarget(0, 0, 1);
		func::mult4x4(vTarget, x_mat, vLookDir);
		func::mult4x4(vLookDir, y_mat, vLookDir);
		vTarget = camera + vLookDir;

		/*
		sf::Vector3f lookDir;
		func::mult4x4(cam_tar, y_mat, lookDir);
		func::mult4x4(lookDir, x_mat, lookDir);
		cam_tar = lookDir + camera;

		*/
		sf::Vector3f up(0, 1, 0);
		sf::Vector3f forward = func::norm3f(vTarget - camera);
		forward = func::norm3f(forward);
		float temp = func::dotpro(up, forward);
		sf::Vector3f a(temp * forward.x, temp * forward.y, temp * forward.z);
		sf::Vector3f newUp = func::norm3f(up - a);
		sf::Vector3f cam_right = func::crossv(func::norm3f(newUp), forward);
		


		c2w[0][0] = cam_right.x; c2w[0][1] = cam_right.y; c2w[0][2] = cam_right.z;
		c2w[1][0] = newUp.x; c2w[1][1] = newUp.y; c2w[1][2] = newUp.z;
		c2w[2][0] = forward.x; c2w[2][1] = forward.y; c2w[2][2] = forward.z;
		c2w[3][0] = camera.x; c2w[3][1] = camera.y; c2w[3][2] = camera.z;
		c2w[3][3] = 1;


		inv[0][0] = c2w[0][0]; inv[0][1] = c2w[1][0]; inv[0][2] = c2w[2][0]; inv[0][3] = 0.0f;
		inv[1][0] = c2w[0][1]; inv[1][1] = c2w[1][1]; inv[1][2] = c2w[2][1]; inv[1][3] = 0.0f;
		inv[2][0] = c2w[0][2]; inv[2][1] = c2w[1][2]; inv[2][2] = c2w[2][2]; inv[2][3] = 0.0f;
		inv[3][0] = -(c2w[3][0] * inv[0][0] + c2w[3][1] * inv[1][0] + c2w[3][2] * inv[2][0]);
		inv[3][1] = -(c2w[3][0] * inv[0][1] + c2w[3][1] * inv[1][1] + c2w[3][2] * inv[2][1]);
		inv[3][2] = -(c2w[3][0] * inv[0][2] + c2w[3][1] * inv[1][2] + c2w[3][2] * inv[2][2]);
		inv[3][3] = 1.0f;

		theta += .1;
		if (theta > 360)
			theta = 0;

		std::vector<Triangle> preparedTriangles;
		for (auto& triangle : rasterizer.triangles)
		{
			sf::Vector3f view_verts[3];

			func::mult4x4(triangle.verts[0], inv, view_verts[0]);
			func::mult4x4(triangle.verts[1], inv, view_verts[1]);
			func::mult4x4(triangle.verts[2], inv, view_verts[2]);

			rasterizer.clip_triangle_near(view_verts, preparedTriangles);
		}

		std::vector<std::vector<float>> z_buffer((float)rasterizer.w_width, std::vector<float>(rasterizer.w_height, INT_MAX));

		for (auto& triangle : preparedTriangles)
		{
			sf::Vector3f proj_verts[3];
			rasterizer.project_triangle(triangle.verts, rasterizer.p_mat, proj_verts);

			proj_verts[0].z = 1 / proj_verts[0].z;
			proj_verts[1].z = 1 / proj_verts[1].z;
			proj_verts[2].z = 1 / proj_verts[2].z;

			triangle.b_left =	std::min({ proj_verts[0].x, proj_verts[1].x, proj_verts[2].x });
			triangle.b_top =	std::min({ proj_verts[0].y, proj_verts[1].y, proj_verts[2].y });
			triangle.b_right =	std::max({ proj_verts[0].x, proj_verts[1].x, proj_verts[2].x });
			triangle.b_bot =	std::max({ proj_verts[0].y, proj_verts[1].y, proj_verts[2].y });

			if (triangle.b_left < rasterizer.w_width && triangle.b_right > -1 && triangle.b_top < rasterizer.w_height - 1 && triangle.b_bot > -1)
			{
				triangle.b_left =	std::max(0, std::min(triangle.b_left, rasterizer.w_width - 1));
				triangle.b_top =	std::max(0, std::min(triangle.b_top, rasterizer.w_height - 1));
				triangle.b_right =	std::min(rasterizer.w_width - 1, std::max(triangle.b_right, 0));
				triangle.b_bot =	std::min(rasterizer.w_height - 1, std::max(triangle.b_bot, 0));
				//std::cout << "left: " << triangle.b_left << " right: " << triangle.b_right << " top: " << triangle.b_top << " bot: " << triangle.b_bot << std::endl;
				
				sf::Vector2f pixel(std::max(triangle.b_left, 0) + .5, std::max(0, triangle.b_top) + .5);
				float a0 = (proj_verts[2].y - proj_verts[1].y), b0 = (proj_verts[1].x - proj_verts[2].x);
				float a1 = (proj_verts[0].y - proj_verts[2].y), b1 = (proj_verts[2].x - proj_verts[0].x);
				float a2 = (proj_verts[1].y - proj_verts[0].y), b2 = (proj_verts[0].x - proj_verts[1].x);

				triangle.area = func::edge_f(sf::Vector2f(proj_verts[0].x, proj_verts[0].y), proj_verts[1], proj_verts[2]);

				/*
				float w0r = func::edge_f(pixel, proj_verts[1], proj_verts[2]);
				float w1r = func::edge_f(pixel, proj_verts[2], proj_verts[0]);
				float w2r = func::edge_f(pixel, proj_verts[0], proj_verts[1]);
				float w0, w1, w2;

				for (int i = triangle.b_top; i < triangle.b_bot; i += 1)
				{
					w0 = w0r;
					w1 = w1r;
					w2 = w2r;

					for (int j = triangle.b_left; j < triangle.b_right; j += 1)
					{
						if ((w0 >= 0 && w1 >= 0 && w2 >= 0) || (w0 < 0 && w1 < 0 && w2 < 0))
						{
							float b0t = w0 / (triangle.area);
							float b1t = w1 / (triangle.area);
							float b2t = w2 / (triangle.area);

							float z = b0t * proj_verts[0].z +
								b1t * proj_verts[1].z +
								b2t * proj_verts[2].z;

							float inv_z = 1 / z;
							if (inv_z < z_buffer[j][i])
							{
								z_buffer[j][i] = inv_z;

								int index = (j + i * 1000) * 4;
								buffer[index] = b0t * 255;
								buffer[index + 1] = b1t * 255;
								buffer[index + 2] = b2t * 255;
								buffer[index + 3] = 255;
							}
						}
						w0 += a0;
						w1 += a1;
						w2 += a2;
					}
					w0r += b0;
					w1r += b1;
					w2r += b2;
				}
				*/
				/**/
				__m256 factor = _mm256_set_ps(7, 6, 5, 4, 3, 2, 1, 0);
				__m256 A0s = _mm256_mul_ps(_mm256_set_ps(a0, a0, a0, a0, a0, a0, a0, a0), factor);
				__m256 A1s = _mm256_mul_ps(_mm256_set_ps(a1, a1, a1, a1, a1, a1, a1, a1), factor);
				__m256 A2s = _mm256_mul_ps(_mm256_set_ps(a2, a2, a2, a2, a2, a2, a2, a2), factor);
				float inc = a0 * 8;
				float inc2 = a1 * 8;
				float inc3 = a2 * 8;
				

				float w0r = func::edge_f(pixel, proj_verts[1], proj_verts[2]);
				float w1r = func::edge_f(pixel, proj_verts[2], proj_verts[0]);
				float w2r = func::edge_f(pixel, proj_verts[0], proj_verts[1]);

				for (int i = triangle.b_top; i < triangle.b_bot; i += 1)
				{
					__m256 w0 = _mm256_add_ps(A0s, _mm256_set_ps(w0r, w0r, w0r, w0r, w0r, w0r, w0r, w0r));
					__m256 w1 = _mm256_add_ps(A1s, _mm256_set_ps(w1r, w1r, w1r, w1r, w1r, w1r, w1r, w1r));
					__m256 w2 = _mm256_add_ps(A2s, _mm256_set_ps(w2r, w2r, w2r, w2r, w2r, w2r, w2r, w2r));

					float* one = (float*)&w0;
					float* two = (float*)&w1;
					float* three = (float*)&w2;

					for (int j = triangle.b_left; j < triangle.b_right; j += 8)
					{					
						for (int k = 0; k < 8 && k + j < triangle.b_right; ++k)
						{
							if ((one[k] >= 0 && two[k] >= 0 && three[k] >= 0) || (one[k] < 0 && two[k] < 0 && three[k] < 0))
							{
								float b0t = one[k] / (triangle.area);
								float b1t = two[k] / (triangle.area);
								float b2t = three[k] / (triangle.area);

								float z = b0t * proj_verts[0].z +
									b1t * proj_verts[1].z +
									b2t * proj_verts[2].z;

								float inv_z = 1 / z;
								if (inv_z < z_buffer[k + j][i])
								{
									z_buffer[j + k][i] = inv_z;

									int index = ((j + k) + i * 1000) * 4;
									buffer[index] = b0t * 255;
									buffer[index + 1] = b1t * 255;
									buffer[index + 2] = b2t * 255;
									buffer[index + 3] = 255;
								}
							}
						}
						w0 = _mm256_add_ps(w0, _mm256_set_ps(inc, inc, inc, inc, inc, inc, inc, inc));
						w1 = _mm256_add_ps(w1, _mm256_set_ps(inc2, inc2, inc2, inc2, inc2, inc2, inc2, inc2));
						w2 = _mm256_add_ps(w2, _mm256_set_ps(inc3, inc3, inc3, inc3, inc3, inc3, inc3, inc3));
					}
					w0r += b0;
					w1r += b1;
					w2r += b2;
				}
				

				/*
				float w0r = func::edge_f(pixel, proj_verts[1], proj_verts[2]);
				float w1r = func::edge_f(pixel, proj_verts[2], proj_verts[0]);
				float w2r = func::edge_f(pixel, proj_verts[0], proj_verts[1]);

				for (int i = triangle.b_top; i < triangle.b_bot; i += 1)
				{
					float w0[8] = { w0r + (a0 * 0), w0r + (a0 * 1), w0r + (a0 * 2), w0r + (a0 * 3),
									w0r + (a0 * 4) , w0r + (a0 * 5) ,w0r + (a0 * 6), w0r + (a0 * 7) };
					float w1[8] = { w1r + (a1 * 0), w1r + (a1 * 1), w1r + (a1 * 2), w1r + (a1 * 3),
									w1r + (a1 * 4) , w1r + (a1 * 5) ,w1r + (a1 * 6), w1r + (a1 * 7) };
					float w2[8] = { w2r + (a2 * 0), w2r + (a2 * 1), w2r + (a2 * 2), w2r + (a2 * 3),
									w2r + (a2 * 4) , w2r + (a2 * 5) ,w2r + (a2 * 6), w2r + (a2 * 7) };

					for (int j = triangle.b_left; j < triangle.b_right; j += 8)
					{
						for (int k = 0; k < 8 && (j + k) < triangle.b_right; ++k)
						{
							if ((w0[k] >= 0 && w1[k] >= 0 && w2[k] >= 0) || (w0[k] < 0 && w1[k] < 0 && w2[k] < 0))
							{
								float b0t = w0[k] / (triangle.area);
								float b1t = w1[k] / (triangle.area);
								float b2t = w2[k] / (triangle.area);

								float z = b0t * proj_verts[0].z +
									b1t * proj_verts[1].z +
									b2t * proj_verts[2].z;

								float inv_z = 1 / z;
								if (inv_z < z_buffer[k + j][i])
								{
									z_buffer[j + k][i] = inv_z;

									int index = ((j + k) + i * 1000) * 4;
									buffer[index] = b0t * 255;
									buffer[index + 1] = b1t * 255;
									buffer[index + 2] = b2t * 255;
									buffer[index + 3] = 255;
								}
							}
						}
						for (int i = 0; i < 8; ++i)
						{
							w0[i] = w0[i] + (a0 * 8);
							w1[i] = w1[i] + (a1 * 8);
							w2[i] = w2[i] + (a2 * 8);
						}
					}
					w0r += b0;
					w1r += b1;
					w2r += b2;
				}*/
								
			}
		}
		//fps counter
		cur_time = clock.getElapsedTime();
		fps = 1.0f / (cur_time.asSeconds() - last_time.asSeconds());
		sf::Text frames(std::to_string((int)fps), font, 50);
		frames.setFillColor(sf::Color::Cyan);
		last_time = cur_time;


		sf::Image image;
		image.create(rasterizer.w_width, rasterizer.w_height, buffer);
		delete[] buffer;

		sf::Texture texture;
		texture.loadFromImage(image);

		sf::Sprite sprite;
		sprite.setTexture(texture);

		window.draw(sprite);
		window.draw(frames);
		window.display();
	}
	return 0;
}