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
	int wWidth, wHeight;
	float cNear, cFar;
	float nWidth, nHeight;
	float fWidth, fHeight;
	float cTop, cRight, cBot, cLeft;

	int fov;
	float aspectRatio;

	float pMat[4][4];


	Rasterizer(int newWWidth, int newWHeight, float newCNear, float newCFar, int newFov);
	bool project_triangle(Triangle& tri, float mat[4][4]);
	void calculateBoundingBox(Triangle& tri);
	void calculateVertexData(Triangle& tri);
	void rasterTile(Tile& tile, std::vector<std::vector<float>>& z_buffer, sf::Uint8*& buffer);
};