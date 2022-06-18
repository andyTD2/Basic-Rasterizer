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
	int wWidth;
	int wHeight;
	float pMat[4][4];

	Rasterizer(int newWindowWidth, int newWindowHeight, int fov, float cNear, float cFar);
	bool project_triangle(Triangle& tri);
	void calculateBoundingBox(Triangle& tri);
	void calculateVertexData(Triangle& tri);
	void rasterTile(Tile& tile, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer);
};