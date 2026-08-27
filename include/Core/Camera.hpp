#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

enum Camera_Movement {
	FORWARD,
	BACKWARD,
	LEFT,
	RIGHT,
	UP,
	DOWN
};

// Default camera values
const float YAW = -90.0f;
const float PITCH = 0.0f;
const float SPEED = 2.5f;
const float SENSITIVITY = 0.1f;
const float ZOOM = 45.0f;

class Camera {
public:
	Camera(glm::vec3 camPosition = glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3 camUp = glm::vec3(0.0f, 1.0f, 0.0f), float camYaw = YAW, float camPitch = PITCH);
	~Camera();

	void Update(float deltaTime);

	void ProcessKeyboard(Camera_Movement direction, float deltaTime);
	void ProcessMouseMovement(float xoffset, float yoffset, GLboolean constrainPitch = true);
	void ProcessMouseScroll(float yoffset);

	glm::mat4 GetViewMatrix() const { return glm::lookAt(position, position + front, up); }
	float GetZoom() const { return zoom; }
	glm::vec3 GetPosition() const { return position; }

private:

	void updateCameraVectors();

	// camera Attributes
	glm::vec3 position;
	glm::vec3 front;
	glm::vec3 up;
	glm::vec3 right;
	glm::vec3 worldUp;
	// euler Angles
	float yaw;
	float pitch;
	// camera options
	float movementSpeed;
	float mouseSensitivity;
	float zoom;


};