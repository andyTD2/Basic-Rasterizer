#include "camera.hpp"
#include "func.hpp"

Camera::Camera()
{
	camPos.x = 0; camPos.y = 0; camPos.z = 0;
	lookDir.x = 0; lookDir.y = 0; lookDir.z = 1;
	curXRotation = 0; curYRotation = 0;
	rotationSpeed = .5; panSpeed = .1;
}

void Camera::updateCamera(bool rotateLeft, bool rotateRight, bool rotateUp, bool rotateDown, 
							bool panForward, bool panBackwards, bool panLeft, bool panRight)
{
	velocity = func::vec3XScalar(lookDir, panSpeed);

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
		sf::Vector3f moveLeft = func::norm3f(func::crossv(velocity, sf::Vector3f(0, 1, 0)));
		moveLeft = func::vec3XScalar(moveLeft, panSpeed);
		camPos += moveLeft;
	}

	if (panRight)
	{
		sf::Vector3f moveRight = func::norm3f(func::crossv(velocity, sf::Vector3f(0, 1, 0)));
		moveRight = func::vec3XScalar(moveRight, panSpeed);
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


	sf::Vector3f target(0, 0, 1);
	func::vecXmatrix(target, x_mat, lookDir);
	func::vecXmatrix(lookDir, y_mat, lookDir);
	target = camPos + lookDir;

	sf::Vector3f up(0, 1, 0);
	sf::Vector3f forward = func::norm3f(target - camPos);
	forward = func::norm3f(forward);
	float temp = func::dotpro(up, forward);
	sf::Vector3f a(temp * forward.x, temp * forward.y, temp * forward.z);
	sf::Vector3f newUp = func::norm3f(up - a);
	sf::Vector3f cam_right = func::crossv(func::norm3f(newUp), forward);

	camMatrix[0][0] = cam_right.x; camMatrix[0][1] = newUp.x; camMatrix[0][2] = forward.x; camMatrix[0][3] = 0.0f;
	camMatrix[1][0] = cam_right.y; camMatrix[1][1] = newUp.y; camMatrix[1][2] = forward.y; camMatrix[1][3] = 0.0f;
	camMatrix[2][0] = cam_right.z; camMatrix[2][1] = newUp.z; camMatrix[2][2] = forward.z; camMatrix[2][3] = 0.0f;
	camMatrix[3][0] = -(camPos.x * camMatrix[0][0] + camPos.y * camMatrix[1][0] + camPos.z * camMatrix[2][0]);
	camMatrix[3][1] = -(camPos.x * camMatrix[0][1] + camPos.y * camMatrix[1][1] + camPos.z * camMatrix[2][1]);
	camMatrix[3][2] = -(camPos.x * camMatrix[0][2] + camPos.y * camMatrix[1][2] + camPos.z * camMatrix[2][2]);
	camMatrix[3][3] = 1.0f;
}

void Camera::setRotationSpeed(float newSpeed)
{
	rotationSpeed = newSpeed;
}

void Camera::setPanSpeed(float newSpeed)
{
	panSpeed = newSpeed;
}