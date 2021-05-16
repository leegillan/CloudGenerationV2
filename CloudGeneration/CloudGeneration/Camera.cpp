#include "Camera.h"

Camera::Camera()
{
	position = { 0.0f, 0.0f, 0.0f };
	rotation = { 0, 0, 0 };
	up = { 0, 1, 0 };
	look = { 0, 0, 1 };

	forward = glm::vec3(0.0f, 0.0f, -1.0f);
	right = glm::normalize(glm::cross(up, forward));
}

void Camera::Update(float deltaTime)
{
	glm::vec3 up, pos, look;
	float yaw, pitch, roll;
	glm::mat4 rotMat;

	pos = { position };
	up = { 0, 1, 0 };

	// Set the yaw (Y axis), pitch (X axis), and roll (Z axis) rotations in radians.
	pitch = glm::radians(rotation.x);
	yaw = glm::radians(rotation.y);
	roll = glm::radians(rotation.z);

	look = { position };
	look.operator+=(forward);
}

void Camera::SetCameraPos(glm::vec3 i)
{
	position = i;

	SetViewMatrix();
}

void Camera::SetCameraLook(glm::vec3 k)
{
	look = k;
}

void Camera::SetCameraUp(glm::vec3 j)
{
	up = j;
}

void Camera::SetForward(glm::vec3 o)
{
	forward = o;
}

glm::vec3 Camera::GetCameraPos()
{
	return position;
}

glm::vec3 Camera::GetCameraRot()
{
	return rotation;
}

glm::vec3 Camera::GetCameraLook()
{
	return look;
}

glm::vec3 Camera::GetCameraUp()
{
	return up;
}

glm::vec3 Camera::GetForward()
{
	return forward;
}