#pragma once
#include <vector>
#include "Scene.hpp"

struct Tile
{
	int bLeft;
	int bTop;
	int bBot;
	int bRight;

	std::vector<Triangle*> trianglesToRender;
};

class TileManager
{
private:
	int tileLengthHorizontal;
	int tileLengthVertical;
public:
	std::vector<std::vector<Tile>> tiles;


	TileManager(int numTiles, int windowWidth, int windowHeight);

	/**
	 * @brief For every tile, add all the triangles that overlap said tile to trianglesToRender
	 *
	 * @param triList: a list of triangles to bin
	 *
	 * @return void; results placed in each tile's trianglesToRender list
	 */
	void binTriangles(const std::vector<Triangle*> triList);
};