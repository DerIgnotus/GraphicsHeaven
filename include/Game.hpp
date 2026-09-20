#pragma once

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>

#include "Core/Camera.hpp"
#include "Core/UI.hpp"
#include "Graphics/Renderer.hpp"
#include "Graphics/Model.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Mesh.hpp"
#include "World/Cubemap.hpp"
#include "World/Sky.hpp"
#include "Core/Window.hpp"

enum GameState {
	GAME_ACTIVE,
	GAME_MENU,
	GAME_PAUSED,
	GAME_END
};

class Game {
public:
	Game(unsigned int width, unsigned int height, const char* name, bool vsyncMode = true);
	~Game();

	int ExtInit();
	int Init();

	void Run();

	const Camera& GetCamera() const { return camera; }
	const GameState& GetGameState() const { return gameState; }
	const float GetDeltaTime() const { return deltaTime; }

	GLFWwindow* GetGLFWWindow() const { return window.GetGLFWWindow(); }

private:
	void update();
	void processInput();

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	double fps = 0.0f;
	double fpsTimer = 0.0f;
	int frameCount = 0;

	bool tabWasPressed = false;

	GameState gameState;

	Camera camera;
	Renderer renderer;
	UI ui;
	Cubemap skybox;
	Sky sky;
	Window window;
	GLFWwindow* glfwWindow;

	Shader modelShader;

	std::vector<Model> models;
};