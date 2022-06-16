#pragma once
#include "SFML/Graphics.hpp"
#include "func.hpp"
#include "Rasterizer.hpp"

class Camera
{
public:
	Camera();
	Camera(float newRotationSpeed, float newPanSpeed);
	void updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
		bool panForward, bool panBackwards, bool panLeft, bool panRight, const Rasterizer& rasterizer);

	void setRotationSpeed(float newSpeed);
	void setPanSpeed(float newSpeed);
	bool checkIfTriangleCulled(const Triangle& triangle) const;
	void transformToViewSpace(Triangle& triangle) const;


	float camMatrix[4][4];
	vec4 camPos;
	vec4 lookDir;
	vec4 velocity;
	vec4 curRight;
	vec4 curUp;
	vec4 curForward;

	//the planes that define our frustum
	vec4 rightNormal;
	vec4 leftNormal;
	vec4 topNormal;
	vec4 botNormal;
private:
	float curXRotation;
	float curYRotation;
	float rotationSpeed;
	float panSpeed;
};