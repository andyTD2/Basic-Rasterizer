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
	std::vector<sf::Vector3f> vertices = { sf::Vector3f(0, 0, 0) };
	std::vector<sf::Vector2f> textureCoords = { sf::Vector2f(0, 0) };
	std::string line;
	std::string firstToken;

	bool lion = false;
	while (!std::getline(file, line).eof())
	{
		std::istringstream iss(line);

		iss >> firstToken;
		if (firstToken == "v")
		{
			float vx, vy, vz;

			iss >> vx >> vy >> vz;
			vertices.push_back(sf::Vector3f(vx, vy, vz));
		}
		else if (firstToken == "o")
		{
			iss >> firstToken;
			if (firstToken == "sponza_377")
				lion = true;
			else lion = false;
		}
		else if (firstToken == "vt")
		{
			float vt0, vt1;
			iss >> vt0 >> vt1;

			if (vt0 < 0)
				vt0 = 0;
			if (vt0 > 1)
				vt0 = 1;
			if (vt1 < 0)
				vt1 = 0;
			if (vt1 > 1)
				vt1 = 1;

			textureCoords.push_back(sf::Vector2f(vt0, vt1));

		}
		else if (lion && firstToken == "f")
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
			sceneData.push_back(Triangle(vertices[v0], vertices[v1], vertices[v2], textureCoords[t0], textureCoords[t1], textureCoords[t2]));
			//if (textureCoords[t0].x > 1 || textureCoords[t0].y > 1 ||
			//	textureCoords[t1].x > 1 || textureCoords[t1].y > 1 ||
			//	textureCoords[t2].x > 1 || textureCoords[t2].y > 1)
			//{
			//	std::cout << "TRIANGLE\n";
			//	std::cout << textureCoords[t0].x << ", " << textureCoords[t0].y << std::endl;
			//	std::cout << textureCoords[t1].x << ", " << textureCoords[t1].y << std::endl;
			//	std::cout << textureCoords[t2].x << ", " << textureCoords[t2].y << std::endl;
			//	std::cout << line << std::endl;
			//}
		}
		
	}
	return true;
}