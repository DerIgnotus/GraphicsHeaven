#include "Core/Window.hpp"
#include <iostream>
#include <glm/ext/matrix_clip_space.hpp>

void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);


Window::Window() : windowWidth(0), windowHeight(0), gameName("No Name!"), vsync(false)
{
}

Window::Window(unsigned int width, unsigned int height, const char* name, bool vsyncMode, Camera* cam) :
	windowWidth(width), windowHeight(height), gameName(name), vsync(vsyncMode), camera(cam)
{
}

Window::~Window()
{
}

int Window::Init()
{
	std::cout << "========================================\n";
	std::cout << "          " << gameName << " - Init\n";
	std::cout << "========================================\n";

	if (!glfwInit()) {
		std::cerr << "[ERROR] Failed to initialize GLFW\n";
		return -1;
	}
	std::cout << "[ OK ] GLFW initialized\n";

	glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
	glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
	glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

	window = glfwCreateWindow(windowWidth, windowHeight, gameName, NULL, NULL);
	if (window == nullptr)
	{
		std::cerr << "[ERROR] Failed to create GLFW window\n";
		glfwTerminate();
		return -1;
	}

	glfwMakeContextCurrent(window);

	glfwSwapInterval(vsync);

	glfwSetWindowUserPointer(window, this);
	glfwSetFramebufferSizeCallback(window, framebuffer_size_callback);
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	cursorLocked = true;

	std::cout << "[ OK ] Window created\n";
	std::cout << "       Resolution: " << windowWidth << "x" << windowHeight << '\n';

	return 0;
}

void Window::SetCursorLocked()
{
	cursorLocked = !cursorLocked;

	glfwSetInputMode(
		window,
		GLFW_CURSOR,
		cursorLocked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
	);
}

const glm::mat4 Window::GetProjMatrix(float cameraZoom)
{
	return glm::perspective(
		glm::radians(cameraZoom),
		(float)windowWidth / (float)windowHeight,
		0.1f,
		100.0f
	);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (windowClass)
		windowClass->OnFramebufferResize(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (windowClass)
		windowClass->OnMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Window* windowClass = static_cast<Window*>(glfwGetWindowUserPointer(window));

	if (windowClass)
		windowClass->OnMouseScroll(xoffset, yoffset);
}


void Window::OnFramebufferResize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	glViewport(0, 0, width, height);
}

void Window::OnMouseMove(double xposIn, double yposIn)
{
	if (!cursorLocked)
		return;

	static bool firstMouse = true;
	static float lastX = 0.0f;
	static float lastY = 0.0f;

	float xpos = static_cast<float>(xposIn);
	float ypos = static_cast<float>(yposIn);

	if (firstMouse)
	{
		lastX = xpos;
		lastY = ypos;
		firstMouse = false;
	}

	float xoffset = xpos - lastX;
	float yoffset = lastY - ypos;

	lastX = xpos;
	lastY = ypos;

	camera->ProcessMouseMovement(xoffset, yoffset);
}

void Window::OnMouseScroll(double xoffset, double yoffset)
{
	camera->ProcessMouseScroll(static_cast<float>(yoffset));
}