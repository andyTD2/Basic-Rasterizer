#pragma once
#include <vector>
#include "math.h"
#include "func.hpp"
#include "Scene.hpp"
#include <iostream>
#include "TileManager.hpp"

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
	bool project_triangle(Triangle& tri, float mat[4][4]);
	int clip_triangle_near(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const;
	void rasterTile(Tile& tile, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer);
};