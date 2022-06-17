#include "Scene.hpp"

bool Scene::loadScene(const std::string& fileName)
{
	//in case we load in a new scene, make sure our sceneData is empty;
	if (!sceneData.empty())
	{
		sceneData.clear();
	}

	std::ifstream file(fileName);

	if (!file.is_open())
		return false;

	//we insert empty element into vector because the index for our vertices and textureCoords starts at 1
	std::vector<vec4> vertices = { vec4(0, 0, 0) };
	std::vector<vec2> textureCoords = { vec2(0, 0) };
	std::string line;
	std::string firstToken;
	std::string curMaterial;

	while (!std::getline(file, line).eof())
	{
		std::istringstream iss(line);

		iss >> firstToken;
		if (firstToken == "mtllib")
		{
			std::string mtlFile;
			iss >> mtlFile;

			loadTexturesFromMtl(mtlFile);
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
	return true;
}

sf::Uint8* Scene::loadTexture(const std::string& fileName, float& returnWidth, float& returnHeight)
{
	std::ifstream file(fileName, std::ios::binary);

	if (file.fail())
	{
		std::cout << fileName << " could not be opened." << std::endl;
		return nullptr;
	}

	unsigned char header[12];
	unsigned short tWidth;
	unsigned short tHeight;
	unsigned char pixelSize;
	unsigned char id;


	file.read((char*)header, sizeof(unsigned char) * 12);
	file.read((char*)&tWidth, sizeof(unsigned short));
	file.read((char*)&tHeight, sizeof(unsigned short));
	file.read((char*)&pixelSize, sizeof(unsigned char));
	file.read((char*)&id, sizeof(unsigned char));


	sf::Uint8* data = nullptr;
	sf::Uint8* dataReversed = nullptr;

	if ((int)pixelSize == 32)
	{
		int tSize = (int)tWidth * (int)tHeight;
		data = new sf::Uint8[tSize * 4];
		dataReversed = new sf::Uint8[tSize * 4];
		file.read((char*)data, tSize * 4);

		int k = 0;
		int rowIncr = tWidth * 4;

		int curRow = 0;
		while (curRow < tSize * 4 - rowIncr)
		{
			for (int i = curRow; i < (curRow + rowIncr); i += 4)
			{
				dataReversed[k] = data[i + 2];
				dataReversed[k + 1] = data[i + 1];
				dataReversed[k + 2] = data[i];
				dataReversed[k + 3] = data[i + 3];
				k += 4;
			}
			curRow += rowIncr;
		}

	}

	else if ((int)pixelSize == 24)
	{
		int tSize = (int)tWidth * (int)tHeight;
		data = new sf::Uint8[tSize * 3];
		dataReversed = new sf::Uint8[tSize * 4];
		file.read((char*)data, tSize * 3);

		int k = 0;

		int rowIncr = tWidth * 3;

		int curRow = 0;
		while (curRow < tSize * 3 - rowIncr)
		{
			for (int i = curRow; i < (curRow + rowIncr); i += 3)
			{
				dataReversed[k] = data[i + 2];
				dataReversed[k + 1] = data[i + 1];
				dataReversed[k + 2] = data[i];
				dataReversed[k + 3] = 255;
				k += 4;
			}
			curRow += rowIncr;
		}

	}

	delete[] data;
	file.close();

	returnWidth = tWidth;
	returnHeight = tHeight;
	return dataReversed;


}

void Scene::loadTexturesFromMtl(const std::string& mtlFile)
{
	std::ifstream file(mtlFile);
	if (file.fail())
	{
		std::cout << mtlFile << " could not be opened." << std::endl;
		return;
	}

	if (!textureData.empty())
		textureData.clear();

	sf::Uint8* placeHolderTexture = new sf::Uint8[1000 * 1000 * 4];
	for (int i = 0; i < 1000 * 1000 * 4; i += 4)
	{
		placeHolderTexture[i] = 200;
		placeHolderTexture[i + 1] = 90;
		placeHolderTexture[i + 2] = 235;
		placeHolderTexture[i + 3] = 255;
	}

	std::string line;
	std::string firstWord;
	std::string mtlName = "";
	std::string textureFileName;
	while (!std::getline(file, line).eof())
	{
		std::istringstream iss(line);
		iss >> firstWord;

		if (firstWord == "newmtl" && mtlName == "")
		{
			iss >> mtlName;
		}
		else if (firstWord == "newmtl" && mtlName != "")
		{
			textureData.insert({ mtlName, placeHolderTexture });
			widthMap.insert({ mtlName, 1000 });
			heightMap.insert({ mtlName, 1000 });
			iss >> mtlName;
		}
		else if (firstWord == "map_Kd" && mtlName != "")
		{
			iss >> textureFileName;

			float w, h;
			textureData.insert({ mtlName, loadTexture(textureFileName, w, h) });
			widthMap.insert({ mtlName, w });
			heightMap.insert({ mtlName, h });
			mtlName = "";
		}
		else if (firstWord == "map_Kd" && mtlName == "")
		{
			std::string bad;
			iss >> bad;
			std::cout << "Error, texture file: " << bad << " not associated with any object name.\n";
		}
	}
	file.close();
	return;
}