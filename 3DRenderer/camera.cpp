#include "camera.hpp"
#include "func.hpp"

Camera::Camera()
{
	camPos.x = 0; camPos.y = 0; camPos.z = 0;
	lookDir.x = 0; lookDir.y = 0; lookDir.z = 1;
	curXRotation = 0; curYRotation = 0;
	rotationSpeed = .5; panSpeed = .1;
}

Camera::Camera(float newRotationSpeed, float newPanSpeed)
{
	camPos.x = 0; camPos.y = 0; camPos.z = 0;
	lookDir.x = 0; lookDir.y = 0; lookDir.z = 1;
	curXRotation = 0; curYRotation = 0;
	rotationSpeed = newRotationSpeed;
	panSpeed = newPanSpeed;
}

void Camera::updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown,
	bool panForward, bool panBackwards, bool panLeft, bool panRight, const Rasterizer& rasterizer)
{
	velocity = lookDir * panSpeed;

	if (panForward)
		camPos += velocity;
	if (panBackwards)
		camPos -= velocity;

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


	float y_mat[4][4] = {
	{cos(curXRotation * 3.14159265359 / 180), 0.0, -sin(curXRotation * 3.14159265359 / 180), 0.0},
	{0.0, 1.0, 0.0, 0.0 },
	{sin(curXRotation * 3.14159265359 / 180), 0.0, cos(curXRotation * 3.14159265359 / 180), 0.0},
	{0.0, 0.0, 0.0, 1.0}
	};

	float x_mat[4][4] = {
	{1.0, 0.0, 0.0, 0.0},
	{0.0, cos(curYRotation * 3.14159265359 / 180), sin(curYRotation * 3.14159265359 / 180), 0.0},
	{0.0, -sin(curYRotation * 3.14159265359 / 180), cos(curYRotation * 3.14159265359 / 180), 0.0},
	{0.0, 0.0, 0.0, 1.0}
	};


	vec4 target(0, 0, 1);
	func::vecXmatrix(target, x_mat, lookDir, false);
	func::vecXmatrix(lookDir, y_mat, lookDir, false);
	target = camPos + lookDir;

	vec4 up(0, 1, 0);
	vec4 forward = func::norm(target - camPos);
	float temp = func::dotPro(up, forward);
	vec4 a(temp * forward.x, temp * forward.y, temp * forward.z);
	vec4 newUp = func::norm(up - a);
	vec4 cam_right = func::crossProd(func::norm(newUp), forward);

	curRight = func::norm(cam_right);
	curForward = func::norm(forward);
	curUp = func::norm(newUp);



	camMatrix[0][0] = cam_right.x; camMatrix[0][1] = newUp.x; camMatrix[0][2] = forward.x; camMatrix[0][3] = 0.0f;
	camMatrix[1][0] = cam_right.y; camMatrix[1][1] = newUp.y; camMatrix[1][2] = forward.y; camMatrix[1][3] = 0.0f;
	camMatrix[2][0] = cam_right.z; camMatrix[2][1] = newUp.z; camMatrix[2][2] = forward.z; camMatrix[2][3] = 0.0f;
	camMatrix[3][0] = -(camPos.x * camMatrix[0][0] + camPos.y * camMatrix[1][0] + camPos.z * camMatrix[2][0]);
	camMatrix[3][1] = -(camPos.x * camMatrix[0][1] + camPos.y * camMatrix[1][1] + camPos.z * camMatrix[2][1]);
	camMatrix[3][2] = -(camPos.x * camMatrix[0][2] + camPos.y * camMatrix[1][2] + camPos.z * camMatrix[2][2]);
	camMatrix[3][3] = 1.0f;


	//now update our frustum normals(necessary for clipping/culling)
	vec4 farCenter = camPos + (curForward * rasterizer.c_far);
	vec4 eUp = curUp * (rasterizer.fHeight / 2);
	vec4 eRight = curRight * (rasterizer.fWidth / 2);
	vec4 farTopRight = farCenter + eUp + eRight;
	vec4 farTopLeft = farCenter + eUp - eRight;
	vec4 farBotRight = farCenter - eUp + eRight;


	vec4 nearCenter = camPos + (curForward * rasterizer.c_near);
	eUp = curUp * (rasterizer.nHeight / 2);
	eRight = curRight * (rasterizer.nWidth / 2);
	vec4 nearTopLeft = nearCenter + eUp - eRight;
	vec4 nearBotLeft = nearCenter - eUp - eRight;
	vec4 nearBotRight = nearCenter - eUp + eRight;

	rightNormal = func::norm(func::crossProd((farTopRight - farBotRight), (nearBotRight - farBotRight)));
	leftNormal = func::norm(func::crossProd((farTopLeft - nearTopLeft), (nearBotLeft - nearTopLeft)));
	topNormal = func::norm(func::crossProd((nearTopLeft - farTopLeft), (farTopRight - farTopLeft)));
	botNormal = func::norm(func::crossProd((nearBotLeft - nearBotRight), (farBotRight - nearBotRight)));
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
	bool culled = false;
	if (func::getDist(rightNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(rightNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(rightNormal, camPos, triangle.verts[2]) < 0)
		culled = true;
	if (func::getDist(leftNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(leftNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(leftNormal, camPos, triangle.verts[2]) < 0)
		culled = true;
	if (func::getDist(topNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(topNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(topNormal, camPos, triangle.verts[2]) < 0)
		culled = true;
	if (func::getDist(botNormal, camPos, triangle.verts[0]) < 0 &&
		func::getDist(botNormal, camPos, triangle.verts[1]) < 0 &&
		func::getDist(botNormal, camPos, triangle.verts[2]) < 0)
		culled = true;

	return culled;
}

void Camera::transformToViewSpace(Triangle& triangle) const
{
	func::vecXmatrix(triangle.verts[0], camMatrix, triangle.transVerts[0], false);
	func::vecXmatrix(triangle.verts[1], camMatrix, triangle.transVerts[1], false);
	func::vecXmatrix(triangle.verts[2], camMatrix, triangle.transVerts[2], false);
}