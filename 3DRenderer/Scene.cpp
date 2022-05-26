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

	std::vector<sf::Vector3f> vertices = { sf::Vector3f(0, 0, 0) };
	std::string line;
	std::string firstToken;

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
		else if (firstToken == "f")
		{
			int v0, v1, v2;

			iss >> v0;
			if (iss.peek() == '/')
				iss.ignore(line.size(), ' ');

			iss >> v1;
			if (iss.peek() == '/')
				iss.ignore(line.size(), ' ');

			iss >> v2;

			sceneData.push_back(Triangle(vertices[v0], vertices[v1], vertices[v2]));
		}
	}
	return true;
}