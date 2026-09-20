#include "Game.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

Game::Game(unsigned int width, unsigned int height, const char* name, bool vsyncMode) :
	gameState(GAME_ACTIVE)
{
	window = Window(width, height, name, vsyncMode, &camera);
}

Game::~Game()
{
	ImGui_ImplOpenGL3_Shutdown();
	ImGui_ImplGlfw_Shutdown();

	ImGui::DestroyContext();

	glfwTerminate();
}

int Game::ExtInit()
{
	// GLFW

	if (window.Init()) {
		return 1;
	}

	glfwWindow = window.GetGLFWWindow();

	//GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "[ERROR] Failed to initialize GLAD\n";
		glfwDestroyWindow(glfwWindow);
		glfwTerminate();
		return -1;
	}

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

	std::cout << "[ OK ] GLAD initialized\n";

	// ImGui
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	if (!ImGui_ImplGlfw_InitForOpenGL(glfwWindow, true))
	{
		std::cerr << "[ERROR] Failed to initialize ImGui GLFW backend\n";
		return -1;
	}

	if (!ImGui_ImplOpenGL3_Init("#version 460"))
	{
		std::cerr << "[ERROR] Failed to initialize ImGui OpenGL3 backend\n";

		ImGui_ImplGlfw_Shutdown();
		ImGui::DestroyContext();

		return -1;
	}

	std::cout << "[ OK ] ImGui initialized\n";

	// OpenGL
	std::cout << "\nOpenGL Information\n";
	std::cout << "----------------------------------------\n";

	std::cout << "  Vendor:   "
		<< glGetString(GL_VENDOR) << '\n';

	std::cout << "  Renderer: "
		<< glGetString(GL_RENDERER) << '\n';

	std::cout << "  OpenGL:   "
		<< glGetString(GL_VERSION) << '\n';

	std::cout << "  GLSL:     "
		<< glGetString(GL_SHADING_LANGUAGE_VERSION) << '\n';

	std::cout << "========================================\n";
	std::cout << "Ext Initialization complete!\n";
	std::cout << "========================================\n\n";

	return 0;
}

int Game::Init()
{
	ui = UI();
	camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	modelShader = Shader("shaders/model2.vert", "shaders/model2.frag");
	//skybox = Cubemap("assets/textures/cubemaps/skybox/");
	sky.SetUp();

	models.emplace_back("assets/models/a1.glb");

	return 0;
}

void Game::Run()
{
	float currentFrame = static_cast<float>(glfwGetTime());
	deltaTime = currentFrame - lastFrame;
	lastFrame = currentFrame;

	fpsTimer += deltaTime;
	frameCount++;

	if (fpsTimer >= 1.0) {
		fps = frameCount / fpsTimer;

		fpsTimer = 0.0;
		frameCount = 0;
	}

	glfwPollEvents();
	processInput();

	glClearColor(0.0f, 0.0f, 0.0f, 1.0f);
	glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

	update();

	glfwSwapBuffers(window.GetGLFWWindow());
}



void Game::update()
{
	glm::mat4 view = camera.GetViewMatrix();

	glm::mat4 projection = window.GetProjMatrix(camera.GetZoom());

	glm::mat4 invViewProjection = glm::inverse(projection * view);

	glm::vec2 windowSize = window.GetWindowSize();

	sky.Render(invViewProjection, windowSize);

	glm::mat4 model = glm::mat4(1.0f);

	modelShader.Use();

	modelShader.SetMat4("model", model);
	modelShader.SetMat4("view", view);
	modelShader.SetMat4("projection", projection);

	modelShader.SetVec3("cameraPos", camera.GetPosition());
	modelShader.SetFloat("reflectionStrength", 0.0f);

	for (const Model& model : models) {
		model.Draw(modelShader, skybox.GetTexture());
	}

	//skybox.Render(view, projection);

	ui.Update(fps, windowSize);
	ui.Render();
}

void Game::processInput()
{
	if (glfwGetKey(glfwWindow, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(glfwWindow, true);
		gameState = GAME_END;
	}

	if (glfwGetKey(glfwWindow, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);

	if (glfwGetKey(glfwWindow, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	if (glfwGetKey(glfwWindow, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);

	if (glfwGetKey(glfwWindow, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(glfwWindow, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);

	if (glfwGetKey(glfwWindow, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	bool tabPressed = glfwGetKey(glfwWindow, GLFW_KEY_TAB) == GLFW_PRESS;

	if (tabPressed && !tabWasPressed)
	{
		window.SetCursorLocked();
	}

	tabWasPressed = tabPressed;
}
