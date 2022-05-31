#pragma once
#include <vector>
#include "func.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

struct Triangle
{
	sf::Vector3f verts[3];
	sf::Vector2f tCoords[3];
	std::string associatedMtl;

	Triangle()
	{

	}
	Triangle(sf::Vector3f v0, sf::Vector3f v1, sf::Vector3f v2, sf::Vector2f t0, sf::Vector2f t1, sf::Vector2f t2, std::string newMtl)
	{
		verts[0] = v0; verts[1] = v1; verts[2] = v2;
		tCoords[0] = t0; tCoords[1] = t1; tCoords[2] = t2;
		associatedMtl = newMtl;
	}
};


class Scene 
{
public:
	std::vector<Triangle> sceneData;
	std::unordered_map<std::string, sf::Uint8*> textureData;
	std::unordered_map<std::string, int> widthMap;
	std::unordered_map<std::string, int> heightMap;


	bool loadScene(const std::string& fileName);
	sf::Uint8* loadTexture(const std::string& fileName, float& returnWidth, float& returnHeight);
	void loadTexturesFromMtl(const std::string& mtlFile);

};