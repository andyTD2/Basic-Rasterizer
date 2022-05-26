#pragma once
#include <vector>
#include "func.hpp"
#include <fstream>
#include <sstream>

class Scene 
{
public:
	std::vector<Triangle> sceneData;

	bool loadScene(const std::string& fileName);

};