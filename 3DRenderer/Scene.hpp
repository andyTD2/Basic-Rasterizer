#pragma once
#include <vector>
#include "func.hpp"
#include <fstream>
#include <sstream>
#include <iostream>
#include <unordered_map>
#include <filesystem>

struct Triangle
{
	float vertexDepth[3];
	float area;

	//vertices
	vec4 verts[3];
	vec4 transVerts[3];
	vec4 projVerts[3];

	//texture information
	vec2 tCoords[3];
	std::string associatedMtl;
	sf::Uint8* triangleTexture;
	int tWidth;
	int tHeight;
	float ty[3];
	float tx[3];

	//bounding box
	int bLeft;
	int bRight;
	int bTop;
	int bBot;

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

inline std::ostream& operator<<(std::ostream& os, const Triangle& tri)
{
	os << "Triangle (" << tri.associatedMtl << ")\n";
	for (int i = 0; i < 3; ++i)
	{
		os << "  Vertex " << i << ": ("
			<< tri.verts[i].x << ", "
			<< tri.verts[i].y << ", "
			<< tri.verts[i].z << ")\n";
		os << "    Trans: ("
			<< tri.transVerts[i].x << ", "
			<< tri.transVerts[i].y << ", "
			<< tri.transVerts[i].z << ")\n";
		os << "    Proj: ("
			<< tri.projVerts[i].x << ", "
			<< tri.projVerts[i].y << ", "
			<< tri.projVerts[i].z << ")\n";
		os << "    Tex: ("
			<< tri.tCoords[i].x << ", "
			<< tri.tCoords[i].y << ")\n";
	}
	os << "  Bounding box: Left=" << tri.bLeft
		<< ", Right=" << tri.bRight
		<< ", Top=" << tri.bTop
		<< ", Bottom=" << tri.bBot << "\n";
	os << "  Area: " << tri.area << "\n";
	os << "  Vertex depths: [" << tri.vertexDepth[0] << ", "
		<< tri.vertexDepth[1] << ", " << tri.vertexDepth[2] << "]\n";
	return os;
}


class Scene
{
public:
	std::vector<Triangle> sceneData;
	std::unordered_map<std::string, sf::Uint8*> textureData;
	std::unordered_map<std::string, int> widthMap;
	std::unordered_map<std::string, int> heightMap;

	/**
	 * @brief Loads all the triangle and texture data in our scene
	 *
	 * The following files must be present for the scene to properly load: .obj, .mtl, texture files. Triangle data
	 * is loaded into sceneData, texture data into textureData, and each texture's width and height values into corresponding
	 * maps. Note: function assumes obj file consists purely of triangles.
	 *
	 * @param fileName: name of obj file
	 * 
	 * @return true if scene properly loaded, false otherwise
	 */
	bool loadScene(const std::string& fileName);

	/**
	 * @brief Loads a TGA texture from file(function deprecated - use stbi_load())
	 *
	 *
	 * @param fileName: name of texture file (tga format only)
	 * @param returnWidth: width value of texture map
	 * @param returnHeight height value of texture map
	 *
	 * @return array containing texel data
	 */
	sf::Uint8* loadTexture(const std::string& fileName, float& returnWidth, float& returnHeight);

	/**
	 * @brief Given an mtl file, load all the textures defined in said file
	 *
	 *
	 * @param fileName: name of mtl file
	 *
	 * @return true if function was able to load all textures
	 */
	bool loadTexturesFromMtl(const std::string& mtlFile);


	friend std::ostream& operator<<(std::ostream& os, const Scene& scene)
	{
		os << "Scene with " << scene.sceneData.size() << " triangles:\n";
		for (size_t i = 0; i < scene.sceneData.size(); ++i)
		{
			os << "Triangle " << i << ":\n";
			os << scene.sceneData[i] << "\n";
		}
		return os;
	}
};
