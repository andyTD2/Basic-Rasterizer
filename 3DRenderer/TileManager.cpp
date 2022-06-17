#include "TileManager.hpp"
#include <mutex>

TileManager::TileManager(int numTiles, int windowWidth, int windowHeight)
{
	tileLengthHorizontal = windowWidth / numTiles;
	tileLengthVertical = windowHeight / numTiles;
	tiles = std::vector<std::vector<Tile>>(numTiles, std::vector<Tile>(numTiles));


	for (int i = 0; i < numTiles; ++i)
	{
		for (int j = 0; j < numTiles; ++j)
		{
			Tile tile;
			tile.bLeft = i * tileLengthHorizontal;
			tile.bRight = tile.bLeft + tileLengthHorizontal;
			tile.bTop = j * tileLengthVertical;
			tile.bBot = tile.bTop + tileLengthVertical;
			tiles[i][j] = tile;
		}
	}

}

void TileManager::binTriangles(std::vector<Triangle*> triList)
{
	__m128 tileDimensions = _mm_set_ps(tileLengthHorizontal, tileLengthHorizontal, tileLengthVertical, tileLengthVertical);
	for (auto& tri : triList)
	{
		__m128 tilePositionsAVX = _mm_div_ps(_mm_set_ps(tri->bLeft, tri->bRight, tri->bTop, tri->bBot), tileDimensions);
		float* tilePositions = (float*)&tilePositionsAVX;

		for (int i = tilePositions[3]; i <= tilePositions[2]; i++)
			for (int j = tilePositions[1]; j <= tilePositions[0]; ++j)
			{
				tiles[i][j].trianglesToRender.push_back(tri);
			}
	}
}