#pragma once
#include <vector>
#include "math.h"
#include "func.hpp"
#include "Scene.hpp"
#include <iostream>

class Rasterizer
{
public:
	int w_width, w_height;
	float c_near, c_far;
	float c_width, c_height;
	float c_top, c_right, c_bot, c_left;

	int fov;
	float aspect_ratio;

	float p_mat[4][4];


	Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int fov);
	void project_triangle(const sf::Vector3f(&tri_verts)[3], float mat[4][4], sf::Vector3f(&proj_verts)[3]);
	void rot_x(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3]);
	void rot_y(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3]);
	void rot_z(Triangle& tri, float degrees, sf::Vector3f(&trans_verts)[3]);
	int clip_triangle_near(const Triangle& tri, sf::Vector3f(&proj_verts)[3], std::vector<Triangle>& out) const;
};