#pragma once
#include "Camera.hpp"

#include <GLFW/glfw3.h>

class Window {
public:
	Window();
	Window(unsigned int width, unsigned int height, const char* name, bool vsyncMode, Camera* cam);
	~Window();

	int Init();

	void SetCursorLocked();

	GLFWwindow* GetGLFWWindow() const { return window; }

	unsigned int GetWindowHeight() const { return windowHeight; }
	unsigned int GetWindowWidth() const { return windowWidth; }
	glm::vec2 GetWindowSize() const { return glm::vec2(windowWidth, windowHeight); }

	const glm::mat4 GetProjMatrix(float cameraZoom);

	void OnFramebufferResize(int width, int height);
	void OnMouseMove(double xpos, double ypos);
	void OnMouseScroll(double xoffset, double yoffset);

private:
	unsigned int windowWidth, windowHeight;
	bool vsync;
	const char* gameName;

	bool cursorLocked = true;

	GLFWwindow* window = nullptr;

	Camera* camera = nullptr;
};