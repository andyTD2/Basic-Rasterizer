#include "camera.hpp"

Camera::Camera()
{
	camPos.x = 0; camPos.y = 0; camPos.z = 0;
	lookDir.x = 0; lookDir.y = 0; lookDir.z = 1;
	curXRotation = 0; curYRotation = 0;
	rotationSpeed = .5; panSpeed = .1;
	cNear = 2; cFar = 1000;
	fov = 60;
}

Camera::Camera(float newRotationSpeed, float newPanSpeed, float newCNear, float newCFar, float newFov) :
	rotationSpeed(newRotationSpeed), panSpeed(newPanSpeed), cNear(newCNear), cFar(newCFar), fov(newFov)
{
	camPos.x = 0; camPos.y = 0; camPos.z = 0;
	lookDir.x = 0; lookDir.y = 0; lookDir.z = 1;
	curXRotation = 0; curYRotation = 0;
}

void Camera::updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
	bool panForward, bool panBackwards, bool panLeft, bool panRight)
{
	velocity = lookDir * panSpeed;

	//updating camera position and direction
	if (panForward) camPos += velocity;
	if (panBackwards) camPos -= velocity;
	if (rotateLeft) curXRotation -= rotationSpeed;
	if (rotateRight) curXRotation += rotationSpeed;
	if (rotateUp) curYRotation -= rotationSpeed;
	if (rotateDown) curYRotation += rotationSpeed;

	if (curXRotation < -360) curXRotation += 360;
	else if (curXRotation > 360) curXRotation -= 360;

	if (curYRotation > 89) curYRotation = 89;
	else if (curYRotation < -89) curYRotation = -89;

	if (panLeft)
	{
		vec4 moveLeft = func::norm(func::crossProd(velocity, vec4(0, 1, 0)));
		moveLeft = moveLeft * panSpeed;
		camPos += moveLeft;
	}

	if (panRight)
	{
		vec4 moveRight = func::norm(func::crossProd(velocity, vec4(0, 1, 0)));
		moveRight = moveRight * panSpeed;
		camPos -= moveRight;
	}

	//a rotation matrix around the x axis
	float yMat[4][4] = {
	{cos(curXRotation * 3.14159265359 / 180), 0.0, -sin(curXRotation * 3.14159265359 / 180), 0.0},
	{0.0, 1.0, 0.0, 0.0 },
	{sin(curXRotation * 3.14159265359 / 180), 0.0, cos(curXRotation * 3.14159265359 / 180), 0.0},
	{0.0, 0.0, 0.0, 1.0}
	};

	float xMat[4][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, cos(curYRotation * 3.14159265359 / 180), sin(curYRotation * 3.14159265359 / 180), 0.0},
	{0.0, -sin(curYRotation * 3.14159265359 / 180), cos(curYRotation * 3.14159265359 / 180), 0.0},
	{0.0, 0.0, 0.0, 1.0}
	};


	vec4 target(0, 0, 1);
	func::vecXmatrixAffine(target, xMat, lookDir);
	func::vecXmatrixAffine(lookDir, yMat, lookDir);
	target = camPos + lookDir;

	//calculation of the up, forward, and right vectors of our camera
	vec4 up(0, 1, 0);
	vec4 forward = func::norm(target - camPos);
	float temp = func::dotPro(up, forward);
	vec4 a(temp * forward.x, temp * forward.y, temp * forward.z);
	vec4 newUp = func::norm(up - a);
	vec4 camRight = func::crossProd(func::norm(newUp), forward);

	curRight = func::norm(camRight);
	curForward = func::norm(forward);
	curUp = func::norm(newUp);

	//our final transform matrix
	camMatrix[0][0] = camRight.x; camMatrix[0][1] = newUp.x; camMatrix[0][2] = forward.x; camMatrix[0][3] = 0.0f;
	camMatrix[1][0] = camRight.y; camMatrix[1][1] = newUp.y; camMatrix[1][2] = forward.y; camMatrix[1][3] = 0.0f;
	camMatrix[2][0] = camRight.z; camMatrix[2][1] = newUp.z; camMatrix[2][2] = forward.z; camMatrix[2][3] = 0.0f;
	camMatrix[3][0] = -(camPos.x * camMatrix[0][0] + camPos.y * camMatrix[1][0] + camPos.z * camMatrix[2][0]);
	camMatrix[3][1] = -(camPos.x * camMatrix[0][1] + camPos.y * camMatrix[1][1] + camPos.z * camMatrix[2][1]);
	camMatrix[3][2] = -(camPos.x * camMatrix[0][2] + camPos.y * camMatrix[1][2] + camPos.z * camMatrix[2][2]);
	camMatrix[3][3] = 1.0f;

	//dimensions of near and far plane
	float nWidth = tan((fov / 2) * 3.14159265359 / 180) * cNear * 2;
	float nHeight = nWidth;
	float fWidth = tan((fov / 2) * 3.14159265359 / 180) * cFar * 2;
	float fHeight = fWidth;

	//points on our near and far plane
	vec4 farCenter = camPos + (curForward * cFar);
	vec4 eUp = curUp * (fHeight / 2);
	vec4 eRight = curRight * (fWidth / 2);
	vec4 farTopRight = farCenter + eUp + eRight;
	vec4 farTopLeft = farCenter + eUp - eRight;
	vec4 farBotRight = farCenter - eUp + eRight;

	vec4 nearCenter = camPos + (curForward * cNear);
	eUp = curUp * (nHeight / 2);
	eRight = curRight * (nWidth / 2);
	vec4 nearTopLeft = nearCenter + eUp - eRight;
	vec4 nearBotLeft = nearCenter - eUp - eRight;
	vec4 nearBotRight = nearCenter - eUp + eRight;

	//normals of the 4 sides of our viewing frustum(pointed inwards)
	rightNormal = func::norm(func::crossProd((farTopRight - farBotRight), (nearBotRight - farBotRight)));
	leftNormal = func::norm(func::crossProd((farTopLeft - nearTopLeft), (nearBotLeft - nearTopLeft)));
	topNormal = func::norm(func::crossProd((nearTopLeft - farTopLeft), (farTopRight - farTopLeft)));
	botNormal = func::norm(func::crossProd((nearBotLeft - nearBotRight), (farBotRight - nearBotRight)));
	nearNormal = func::norm(func::crossProd((nearBotRight - nearBotLeft), (nearTopLeft - nearBotLeft)));
}

void Camera::setRotationSpeed(float newSpeed)
{
	rotationSpeed = newSpeed;
}

void Camera::setPanSpeed(float newSpeed)
{
	panSpeed = newSpeed;
}

bool Camera::checkIfTriangleCulled(const Triangle& triangle) const
{
	if (func::getDist(nearNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(nearNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(nearNormal, camPos, triangle.verts[2]) < 0)
		return true;
	if (func::getDist(rightNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(rightNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(rightNormal, camPos, triangle.verts[2]) < 0)
		return true;
	if (func::getDist(leftNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(leftNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(leftNormal, camPos, triangle.verts[2]) < 0)
		return true;
	if (func::getDist(topNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(topNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(topNormal, camPos, triangle.verts[2]) < 0)
		return true;
	if (func::getDist(botNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(botNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(botNormal, camPos, triangle.verts[2]) < 0)
		return true;

	return false;
}

void Camera::transformToViewSpace(Triangle& triangle) const
{
	func::vecXmatrixAffine(triangle.verts[0], camMatrix, triangle.transVerts[0]);
	func::vecXmatrixAffine(triangle.verts[1], camMatrix, triangle.transVerts[1]);
	func::vecXmatrixAffine(triangle.verts[2], camMatrix, triangle.transVerts[2]);
}

bool Camera::clipTriangleNear(Triangle& tri, std::vector<Triangle*>& outputTris, std::vector<Triangle*>& trisToDelete) const
{
	if (tri.transVerts[0].z >= cNear && tri.transVerts[1].z >= cNear && tri.transVerts[2].z >= cNear)
	{
		outputTris.push_back(&tri);
		return false;
	}

	std::vector<vec4> clippedVertices;
	std::vector<vec4> safeVertices;
	std::vector<vec2> clippedTVertices;
	std::vector<vec2> safeTVertices;

	for (int i = 0; i < 3; ++i)
	{
		if (tri.transVerts[i].z < cNear)
		{
			clippedVertices.push_back(tri.transVerts[i]);
			clippedTVertices.push_back(tri.tCoords[i]);
		}
		else
		{
			safeVertices.push_back(tri.transVerts[i]);
			safeTVertices.push_back(tri.tCoords[i]);
		}
	}

	// only need to create one new triangle
	if (clippedVertices.size() == 2)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, cNear), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, cNear), vec4(0, 0, 1), safeVertices[0], clippedVertices[1], t);
		vec2 newT2;
		newT2.x = safeTVertices[0].x + (t * (clippedTVertices[1].x - safeTVertices[0].x));
		newT2.y = safeTVertices[0].y + (t * (clippedTVertices[1].y - safeTVertices[0].y));

		Triangle* clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[0], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[0];
		clippedTri->triangleTexture = tri.triangleTexture;
		clippedTri->tWidth = tri.tWidth;
		clippedTri->tHeight = tri.tHeight;
		outputTris.push_back(clippedTri);
		trisToDelete.push_back(clippedTri);
	}

	else if (clippedVertices.size() == 1)
	{
		float t;
		vec4 one = func::getIntersection(vec4(0, 0, cNear), vec4(0, 0, 1), safeVertices[0], clippedVertices[0], t);
		vec2 newT1;
		newT1.x = safeTVertices[0].x + (t * (clippedTVertices[0].x - safeTVertices[0].x));
		newT1.y = safeTVertices[0].y + (t * (clippedTVertices[0].y - safeTVertices[0].y));

		vec4 two = func::getIntersection(vec4(0, 0, cNear), vec4(0, 0, 1), safeVertices[1], clippedVertices[0], t);
		vec2 newT2;
		newT2.x = safeTVertices[1].x + (t * (clippedTVertices[0].x - safeTVertices[1].x));
		newT2.y = safeTVertices[1].y + (t * (clippedTVertices[0].y - safeTVertices[1].y));

		Triangle* clippedTri = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, newT2, safeTVertices[1], tri.associatedMtl);
		clippedTri->transVerts[0] = one;
		clippedTri->transVerts[1] = two;
		clippedTri->transVerts[2] = safeVertices[1];
		clippedTri->triangleTexture = tri.triangleTexture;
		clippedTri->tWidth = tri.tWidth;
		clippedTri->tHeight = tri.tHeight;

		Triangle* clippedTri2 = new Triangle(tri.verts[0], tri.verts[1], tri.verts[2], newT1, safeTVertices[0], safeTVertices[1], tri.associatedMtl);
		clippedTri2->transVerts[0] = one;
		clippedTri2->transVerts[1] = safeVertices[0];
		clippedTri2->transVerts[2] = safeVertices[1];
		clippedTri2->triangleTexture = tri.triangleTexture;
		clippedTri2->tWidth = tri.tWidth;
		clippedTri2->tHeight = tri.tHeight;
		outputTris.push_back(clippedTri);
		outputTris.push_back(clippedTri2);
		trisToDelete.push_back(clippedTri);
		trisToDelete.push_back(clippedTri2);
	}
	return true;
}