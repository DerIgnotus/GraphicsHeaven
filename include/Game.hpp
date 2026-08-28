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

	void OnFramebufferResize(int width, int height);
	void OnMouseMove(double xpos, double ypos);
	void OnMouseScroll(double xoffset, double yoffset);


	const Camera& GetCamera() const { return camera; }
	const GameState& GetGameState() const { return gameState; }
	const float GetDeltaTime() const { return deltaTime; }

	GLFWwindow* GetWindow() const { return window; }

private:
	void update();
	void processInput();
	void setCursorLocked(bool locked);

	unsigned int windowWidth, windowHeight;
	const char* gameName;

	float deltaTime = 0.0f;
	float lastFrame = 0.0f;
	double fps = 0.0f;
	double fpsTimer = 0.0f;
	int frameCount = 0;

	bool cursorLocked = true;
	bool tabWasPressed = false;

	bool vsync;

	GameState gameState;

	Camera camera;
	Renderer renderer;
	UI ui;
	Cubemap skybox;

	GLFWwindow* window = nullptr;

	Shader modelShader;

	std::vector<Model> models;
};