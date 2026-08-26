#include "Game.hpp"

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

void framebuffer_size_callback(GLFWwindow* window, int width, int height);

Game::Game(unsigned int width, unsigned int height, const char* name, bool vsyncMode) :
	windowWidth(width), windowHeight(height), gameState(GAME_ACTIVE), gameName(name), vsync(vsyncMode)
{

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
	std::cout << "========================================\n";
	std::cout << "          " << gameName << " - Init\n";
	std::cout << "========================================\n";

	// GLFW
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

	std::cout << "[ OK ] Window created\n";
	std::cout << "       Resolution: " << windowWidth << "x" << windowHeight << '\n';

	//GLAD
	if (!gladLoadGLLoader((GLADloadproc)glfwGetProcAddress))
	{
		std::cerr << "[ERROR] Failed to initialize GLAD\n";
		glfwDestroyWindow(window);
		glfwTerminate();
		return -1;
	}

	std::cout << "[ OK ] GLAD initialized\n";

	// ImGui
	IMGUI_CHECKVERSION();

	ImGui::CreateContext();

	ImGuiIO& io = ImGui::GetIO();
	(void)io;

	ImGui::StyleColorsDark();

	if (!ImGui_ImplGlfw_InitForOpenGL(window, true))
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
	Model test_model("assets/models/test_crystal.glb");

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
	glClear(GL_COLOR_BUFFER_BIT);

	update();

	glfwSwapBuffers(window);
}



void Game::update()
{
	ui.Update(fps, windowWidth, windowHeight);
	ui.Render();
}

void Game::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS) {
		glfwSetWindowShouldClose(window, true);
		gameState = GAME_END;
	}
}

void Game::OnFramebufferResize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	glViewport(0, 0, width, height);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

	if (game)
		game->OnFramebufferResize(width, height);
}