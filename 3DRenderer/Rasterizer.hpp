#pragma once
#include <vector>
#include "func.hpp"
#include "Scene.hpp"
#include "TileManager.hpp"

class Rasterizer
{
public:
	int wWidth;
	int wHeight;
	float pMat[4][4];

	Rasterizer(int newWindowWidth, int newWindowHeight, int fov, float cNear, float cFar);

	/**
	 * @brief Performs perspective division on a triangle. Places results in triangle.projVerts
	 *
	 * @param tri: the input triangle to project
	 *
	 * @return void; results placed in triangle.projVerts
	 */
	void projectTriangle(Triangle& tri) const;

	/**
	 * @brief Calculates the bounding box of the triangle's projected vertices.
	 *
	 * Results placed in tri.bLeft, tri.bRight, tri.bTop, tri.bBot
	 * 
	 * @param tri: the input triangle that contains the projected verts to calculate.
	 * 
	 * @return void
	 */
	void calculateBoundingBox(Triangle& tri) const;

	/**
	 * @brief Calculates triangle area, and vertex depth + texture data
	 *
	 * @param tri: the input triangle
	 *
	 * @return void; results placed in triangle.tx, ty, area, & vertexDepth
	 */
	void calculateVertexData(Triangle& tri) const;

	/**
	 * @brief Calculates and sets the pixel data of the region defined by a tile
	 *
	 * Given a tile, this function will process all pixels within each triangle whose bounding box overlaps said tile.
	 * If pixel is visible, draw said pixel data(format RGBA) to buffer
	 * 
	 * @param tile: the tile to process
	 * @param zBuffer: the depth buffer
	 * @param pixelBuffer: buffer that holds all the pixel data
	 *
	 * @return void; results placed in zBuffer and pixelBuffer
	 */
	void rasterTile(const Tile& tile, std::vector<std::vector<float>>& zBuffer, sf::Uint8*& pixelBuffer) const;
};