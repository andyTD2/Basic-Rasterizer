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
	float nWidth, nHeight;
	float fWidth, fHeight;
	float c_top, c_right, c_bot, c_left;

	int fov;
	float aspect_ratio;

	float p_mat[4][4];


	Rasterizer(int _w_width, int _w_height, float _c_near, float _c_far, int fov);
	bool project_triangle(Triangle& tri, float mat[4][4], std::vector<Triangle*>& out);
	//int clip_triangle_near(const Triangle& tri, vec4(&proj_verts)[3], std::vector<Triangle>& out) const;
	int clip_triangle_near(const Triangle& tri, vec4(&proj_verts)[3], std::vector<Triangle*>& out) const;
};