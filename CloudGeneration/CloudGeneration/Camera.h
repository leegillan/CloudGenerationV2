#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtx/transform.hpp>
#include <glm/gtc/type_ptr.hpp>

class Camera
{
public:
	Camera();

	void Update(float deltaTime);

	void SetCameraPos(glm::vec3);
	void SetRotation(glm::vec3);
	void SetCameraLook(glm::vec3);
	void SetCameraUp(glm::vec3);
	void SetForward(glm::vec3);

	void SetProjectionMatrix(const float& width, const float& height, float nearPlane, float farPlane) { projectionMatrix = glm::perspective(glm::radians(60.0f), width / height, nearPlane, farPlane); }
	void SetViewMatrix() { viewMatrix = glm::lookAt(position, look, up); }

	glm::vec3 GetCameraPos();
	glm::vec3 GetCameraRot();
	glm::vec3 GetCameraLook();
	glm::vec3 GetCameraUp();
	glm::vec3 GetForward();

	const glm::mat4& GetProjectionMatrix() { return projectionMatrix; }
	const glm::mat4& GetViewMatrix() { return viewMatrix; };

private:
	glm::mat4 projectionMatrix;
	glm::mat4 viewMatrix;
	glm::mat4 ViewProjectionMatrix;

	glm::vec3 position;
	glm::vec3 rotation;
	glm::vec3 up;
	glm::vec3 look;

	glm::vec3 forward;
	glm::vec3 right;
};

