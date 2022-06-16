#pragma once
#include <vector>
#include "func.hpp"
#include <fstream>
#include <sstream>
#include <unordered_map>

struct Triangle
{
	float vertexDepth[3];
	sf::Uint8* triangleTexture;
	int tWidth;
	int tHeight;
	float ty[3];
	float tx[3];
	float area;

	vec4 verts[3];
	vec4 transVerts[3];
	vec4 projVerts[3];
	vec2 tCoords[3];
	std::string associatedMtl;

	int bLeft;
	int bRight;
	int bTop;
	int bBot;

	Triangle()
	{
	}
	Triangle(const Triangle& other)
	{
		for (int i = 0; i < 3; ++i)
		{
			this->verts[i] = other.verts[i];
			this->projVerts[i] = other.projVerts[i];
			this->tCoords[i] = other.tCoords[i];
		}
		this->associatedMtl = other.associatedMtl;
		this->bLeft = other.bLeft;
		this->bRight = other.bRight;
		this->bBot = other.bBot;
		this->bTop = other.bTop;
	}
	Triangle(vec4 v0, vec4 v1, vec4 v2, vec2 t0, vec2 t1, vec2 t2, std::string newMtl)
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