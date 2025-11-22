#include "Scene.hpp"
#include "stb_image.h"

//#include <filesystem>
bool Scene::loadScene(const std::string& fileName)
{
	//in case we load in a new scene, make sure our sceneData is empty;
	if (!sceneData.empty())
	{
		sceneData.clear();
	}

	std::ifstream file(fileName);

	if (!file.is_open())
	{
		//std::cout << std::filesystem::absolute(fileName) << "\n";
		std::cout << "Failed to load file: " << fileName << "\n";
		return false;
	}

	//we insert empty element into vector because the index for our vertices and textureCoords starts at 1
	std::vector<vec4> vertices = { vec4(0, 0, 0) };
	std::vector<vec2> textureCoords = { vec2(0, 0) };
	std::string line;
	std::string firstToken;
	std::string curMaterial;

	bool foundMtlFile = false;
	while (!std::getline(file, line).eof())
	{
		std::istringstream iss(line);

		//read in first token of each line and skip empty/invalid lines
		if (!(iss >> firstToken))
			continue;

		if (firstToken == "mtllib")
		{
			std::string mtlFile;
			iss >> mtlFile;

			if (loadTexturesFromMtl("obj/" + mtlFile))
				foundMtlFile = true;
			else
				return false;
		}
		else if (firstToken == "v")
		{
			float vx, vy, vz;

			iss >> vx >> vy >> vz;
			vertices.push_back(vec4(vx, vy, vz));
		}
		else if (firstToken == "usemtl")
		{
			iss >> curMaterial;
		}
		else if (firstToken == "vt")
		{
			float vt0, vt1;
			iss >> vt0 >> vt1;

			textureCoords.push_back(vec2(vt0, vt1));

		}
		else if (firstToken == "f")
		{

			int v0, v1, v2;
			int t0, t1, t2;

			//std::cout << line << std::endl;
			iss >> v0;
			if (iss.peek() == '/')
			{
				iss.ignore(line.size(), '/');
				iss >> t0;
				iss.ignore(line.size(), ' ');
			}

			iss >> v1;
			if (iss.peek() == '/')
			{
				iss.ignore(line.size(), '/');
				iss >> t1;
				iss.ignore(line.size(), ' ');
			}

			iss >> v2;
			if (iss.peek() == '/')
			{
				iss.ignore(line.size(), '/');
				iss >> t2;
			}

			sceneData.push_back(Triangle(vertices[v0], vertices[v1], vertices[v2], textureCoords[t0], textureCoords[t1], textureCoords[t2], curMaterial));
		}

	}
	for (auto& tri : sceneData)
	{
		tri.triangleTexture = textureData.find(tri.associatedMtl)->second;
		tri.tWidth = widthMap.find(tri.associatedMtl)->second;
		tri.tHeight = heightMap.find(tri.associatedMtl)->second;
	}

	if (!foundMtlFile)
	{
		std::cout << "Could not find MTL file" << std::endl;
		return false;
	}

	return true;
}
bool Scene::loadTexturesFromMtl(const std::string& mtlFile)
{
	std::cout << "loading MTL file:" << mtlFile << std::endl;
	std::ifstream file(mtlFile);
	if (file.fail())
	{
		std::cout << mtlFile << " could not be opened." << std::endl;
		return false;
	}

	if (!textureData.empty())
		textureData.clear();

	sf::Uint8* placeHolderTexture = new sf::Uint8[1000 * 1000 * 4];
	for (int y = 0; y < 1000; ++y)
	{
		for (int x = 0; x < 1000; ++x)
		{
			int index = (y * 1000 + x) * 4;

			// Example: diagonal gradient
			placeHolderTexture[index] = static_cast<sf::Uint8>((x + y) / 2000.0f * 255); // R channel
			placeHolderTexture[index + 1] = static_cast<sf::Uint8>((1000 - x + y) / 2000.0f * 255); // G channel
			placeHolderTexture[index + 2] = static_cast<sf::Uint8>(255 - ((x + y) / 2000.0f * 255)); // B channel
			placeHolderTexture[index + 3] = 255; // Alpha
		}
	}

	std::string line;
	std::string firstWord;
	std::string mtlName = "";
	std::string textureFileName;

	while (!std::getline(file, line).eof())
	{
		std::istringstream iss(line);
		if (!(iss >> firstWord)) continue; // skip empty lines

		if (firstWord == "newmtl" && mtlName == "")
		{
			iss >> mtlName;
		}
		else if (firstWord == "newmtl" && mtlName != "")
		{
			// Assign placeholder to previous material if it has no texture
			if (textureData.find(mtlName) == textureData.end())
			{
				textureData[mtlName] = placeHolderTexture;
				widthMap[mtlName] = 1000;
				heightMap[mtlName] = 1000;
			}

			iss >> mtlName;
		}
		else if (firstWord == "map_Kd" && mtlName != "")
		{
			iss >> textureFileName;
			int w, h, n;

			textureFileName = textureFileName.substr(textureFileName.find_last_of("/\\") + 1);
			textureFileName = "obj/textures2/" + textureFileName;
			std::cout << "Loading texture file: " << textureFileName << std::endl;

			stbi_set_flip_vertically_on_load(1);
			sf::Uint8* tData = stbi_load(textureFileName.c_str(), &w, &h, &n, 4);
			if (tData == nullptr)
			{
				std::cout << "Warning: could not load texture file: " << textureFileName
					<< ". Using placeholder." << std::endl;
				tData = placeHolderTexture;
				w = h = 1000;
			}
			textureData[mtlName] = tData;
			widthMap[mtlName] = w;
			heightMap[mtlName] = h;

			// Optional: reset mtlName if you only expect one map per material
			// mtlName = "";
		}
		else if (firstWord == "map_Kd" && mtlName == "")
		{
			std::string bad;
			iss >> bad;
			std::cout << "Warning: texture file: " << bad
				<< " not associated with any object name." << std::endl;
		}
	}

	// Handle the last material if it has no texture
	if (!mtlName.empty() && textureData.find(mtlName) == textureData.end())
	{
		textureData[mtlName] = placeHolderTexture;
		widthMap[mtlName] = 1000;
		heightMap[mtlName] = 1000;
	}

	file.close();
	return true;
}
