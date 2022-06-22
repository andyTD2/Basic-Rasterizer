#pragma once
#include "SFML/Graphics.hpp"
#include "func.hpp"
#include "Scene.hpp"

class Camera
{
public:
	Camera();
	Camera(float newRotationSpeed, float newPanSpeed, float newCNear, float newCFar, float newFov);

	/**
	 * @brief Updates camera position
	 *
	 * Calculates our new position, transform matrix(cam.camMatrix) and view frustum to be used by other functions.
	 * It takes in booleans to determine how the camera moves and is updated once per frame. You can change the speed
	 * per frame by using setRotationSpeed() and setPanSpeed()
	 *
	 * @param rotateLeft: determines whether camera will rotate towards the left(degrees determined by setRotationSpeed())
	 * @param panForward: determines whether camera will move forward(amount determined by setPanSpeed())
	 *	etc...
	 *
	 * @return void
	 */
	void updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
		bool panForward, bool panBackwards, bool panLeft, bool panRight);

	/**
	 * @brief  Checks if said triangle is outside of the viewing frustum. If it is, return true. Does not cull against far plane
	 *
	 * @param  triangle: triangle to be checked
	 *
	 * @return True if triangle is outside viewing frustum, else false
	 */
	bool checkIfTriangleCulled(const Triangle& triangle) const;

	/**
	 * @brief  Clips triangles that are partially inside the near plane
	 *
	 * Triangles that intersect the near plane have to be clipped to avoid division by 0. If we find that the triangle partially
	 * intersects the near plane, we clip it, producing 1 or 2 new triangles. We then push our new triangles to outputTris and
	 * trisToDelete. trisTodelete is a list of pointers to only the triangles that have been produced as a result of clipping
	 * which allows us to easily delete them later. If the triangle does not intersect the near plane, make no changes and push
	 * to outputTris
	 *
	 * @param  tri: input triangle
	 * @param  outputTris:  a list of triangles which we output that are visible(behind our near plane). This includes new triangles
	 *						that have been produced by clipping.
	 * @param trisToDelete: a list of new triangles produced by the clipping algorithm
	 *
	 * @return true if clipping occurred, false otherwise
	 */
	bool clipTriangleNear(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const;

	//multiplies each vertex of given triangle by camMatrix to transform to view space. Results found in triangle.transVerts
	void transformToViewSpace(Triangle& triangle) const;
	void setRotationSpeed(float newSpeed);
	void setPanSpeed(float newSpeed);


	float cNear, cFar;
	float fov;

private:
	float curXRotation;
	float curYRotation;
	float rotationSpeed;
	float panSpeed;

	//tranform matrix
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
	vec4 nearNormal;
};