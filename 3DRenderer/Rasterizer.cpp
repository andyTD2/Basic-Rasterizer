#pragma once
#include "Rasterizer.hpp"
#include "func.hpp"
#include <iostream>



Rasterizer::Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int _fov) :
	w_width(_w_width), w_height(_w_height), c_near(_c_near), c_far(_c_far), fov(_fov)
{
	const float pi = 3.141592653;
	aspect_ratio = w_width / w_height;

	//canvas size and dimensions
	c_width = tan((fov / 2) * pi / 180) * c_near * 2;
	c_height = c_width;
	c_top = c_height / 2;
	c_right = c_width / 2;
	c_bot = -c_top;
	c_left = -c_right;

	for (int i = 0; i < 4; ++i)
	{
		for (int j = 0; j < 4; ++j)
		{
			p_mat[i][j] = 0;
		}
	}

	//projection matrix
	p_mat[0][0] = aspect_ratio * 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	p_mat[1][1] = 1.0 / tan(fov * 0.5f / 180.0f * 3.14159f);
	p_mat[2][2] = c_far / (c_far - c_near);
	p_mat[2][3] = 1;
	p_mat[3][2] = (-c_far * c_near) / (c_far - c_near);
	p_mat[3][3] = 0;

}

void Rasterizer::project_triangle(const sf::Vector3f(&tri_verts)[3], float mat[4][4], sf::Vector3f (&proj_verts)[3])
{
	func::mult4x4(tri_verts[0], mat, proj_verts[0]);
	func::mult4x4(tri_verts[1], mat, proj_verts[1]);
	func::mult4x4(tri_verts[2], mat, proj_verts[2]);

	for (int i = 0; i < 3; ++i)
	{
		proj_verts[i].x = floor((proj_verts[i].x + 1) / 2 * w_width);
		proj_verts[i].y = floor((1 - proj_verts[i].y) / 2 * w_height);
	}
	return;
}

void Rasterizer::rot_x(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float x_mat[4][4] ={
		{1.0, 0.0, 0.0, 0.0},
		{0.0, cos(degrees * 3.14159265359 / 180), sin(degrees * 3.14159265359 / 180), 0.0},
		{0.0, -sin(degrees * 3.14159265359 / 180), cos(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};

	func::mult4x4(tri.verts[0], x_mat, trans_verts[0]);
	func::mult4x4(tri.verts[1], x_mat, trans_verts[1]);
	func::mult4x4(tri.verts[2], x_mat, trans_verts[2]);

	return;
}

void Rasterizer::rot_y(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float y_mat[4][4] = {
		{cos(degrees * 3.14159265359 / 180), 0.0, -sin(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 1.0, 0.0, 0.0 },
		{sin(degrees * 3.14159265359 / 180), 0.0, cos(degrees * 3.14159265359 / 180), 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};

	func::mult4x4(tri.verts[0], y_mat, trans_verts[0]);
	func::mult4x4(tri.verts[1], y_mat, trans_verts[1]);
	func::mult4x4(tri.verts[2], y_mat, trans_verts[2]);
	return;
}
void Rasterizer::rot_z(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3])
{
	float z_mat[4][4] = {
		{cos(degrees * 3.14159265359 / 180), sin(degrees * 3.14159265359 / 180), 0.0, 0.0},
		{-sin(degrees * 3.14159265359 / 180), cos(degrees * 3.14159265359 / 180), 0.0, 0.0},
		{0.0, 0.0, 1.0, 0.0},
		{0.0, 0.0, 0.0, 1.0}
	};

	func::mult4x4(tri.verts[0], z_mat, trans_verts[0]);
	func::mult4x4(tri.verts[1], z_mat, trans_verts[1]);
	func::mult4x4(tri.verts[2], z_mat, trans_verts[2]);

	return;
}

int Rasterizer::clip_triangle_near(sf::Vector3f(&proj_verts)[3], std::vector<Triangle>& out)
{
	std::vector<sf::Vector3f> clipped_vertices;
	std::vector<sf::Vector3f> safe_vertices;
	//std::cout << "CLIPPING\n";
	//func::print(proj_verts[0]);
	//func::print(proj_verts[1]);
	//func::print(proj_verts[2]);


	for (int i = 0; i < 3; ++i)
	{
		if (proj_verts[i].z < c_near)
			clipped_vertices.push_back(proj_verts[i]);
		else safe_vertices.push_back(proj_verts[i]);
	}


	if (clipped_vertices.size() == 3)
	{
		return 0;
	}

	///VVVVVVVVVV EXCEPTION BEING CAUSED HERE
	// no vertex has crossed the near plane, do nothing
	if (clipped_vertices.size() == 0)
	{
		out.push_back(Triangle(proj_verts[0], proj_verts[1], proj_verts[2]));
		return 0;
	}

	// only need to create one new triangle
	else if (clipped_vertices.size() == 2)
	{
		//std::cout << "new tri\n";
		sf::Vector3f one = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safe_vertices[0], clipped_vertices[0]);
		sf::Vector3f two = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safe_vertices[0], clipped_vertices[1]);
		out.push_back(Triangle(one, two, safe_vertices[0]));
		return 1;
	}

	else if (clipped_vertices.size() == 1)
	{
		//std::cout << "2x new tri\n";
		sf::Vector3f one = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safe_vertices[0], clipped_vertices[0]);
		sf::Vector3f two = func::getIntersection(sf::Vector3f(0, 0, c_near), sf::Vector3f(0, 0, 1), safe_vertices[1], clipped_vertices[0]);
		out.push_back(Triangle(one, two, safe_vertices[1]));
		out.push_back(Triangle(one, safe_vertices[0], safe_vertices[1]));
		return 2;
	}
	return 0;


}
