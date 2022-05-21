#pragma once
#include "SFML/Graphics.hpp"

class Camera
{
public:
	Camera();
	void updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
		bool panForward, bool panBackwards, bool panLeft, bool panRight);

	void setRotationSpeed(float newSpeed);
	void setPanSpeed(float newSpeed);

	float camMatrix[4][4];
	sf::Vector3f camPos;

private:
	sf::Vector3f lookDir;
	sf::Vector3f velocity;
	float curXRotation;
	float curYRotation;
	float rotationSpeed;
	float panSpeed;
};