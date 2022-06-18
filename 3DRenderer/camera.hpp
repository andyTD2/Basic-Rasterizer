#pragma once
#include "SFML/Graphics.hpp"
#include "func.hpp"
#include "Rasterizer.hpp"

class Camera
{
public:
	Camera();
	Camera(float newRotationSpeed, float newPanSpeed, float newCNear, float newCFar, float newFov);

	void updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
		bool panForward, bool panBackwards, bool panLeft, bool panRight);
	void setRotationSpeed(float newSpeed);
	void setPanSpeed(float newSpeed);
	bool checkIfTriangleCulled(const Triangle& triangle) const;
	void transformToViewSpace(Triangle& triangle) const;
	int clipTriangleNear(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const;


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

	float cNear, cFar;
	float fov;
private:
	float curXRotation;
	float curYRotation;
	float rotationSpeed;
	float panSpeed;
};