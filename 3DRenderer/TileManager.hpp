#pragma once
#include <vector>
#include "Scene.hpp"

struct Tile
{
	int bLeft;
	int bTop;
	int bBot;
	int bRight;

	float maxZ = FLT_MAX;
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
	void binTriangles(std::vector<Triangle*> triList);
};