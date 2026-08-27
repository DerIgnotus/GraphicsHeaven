#include "Game.hpp"
#include <glm/gtc/type_ptr.hpp>

#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"


void framebuffer_size_callback(GLFWwindow* window, int width, int height);
void mouse_callback(GLFWwindow* window, double xpos, double ypos);
void scroll_callback(GLFWwindow* window, double xoffset, double yoffset);

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
	glfwSetCursorPosCallback(window, mouse_callback);
	glfwSetScrollCallback(window, scroll_callback);

	glfwSetInputMode(window, GLFW_CURSOR, GLFW_CURSOR_DISABLED);
	cursorLocked = true;


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

	glEnable(GL_DEPTH_TEST);
	glDisable(GL_CULL_FACE);

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
	camera = Camera(glm::vec3(0.0f, 0.0f, 3.0f));
	modelShader = Shader("shaders/model.vert", "shaders/model.frag");

	models.emplace_back("assets/models/lambo2.glb");

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

	glfwSwapBuffers(window);
}



void Game::update()
{
	modelShader.Use();

	glm::mat4 model = glm::mat4(1.0f);
	glm::mat4 view = camera.GetViewMatrix();

	glm::mat4 projection = glm::perspective(
		glm::radians(camera.GetZoom()),
		(float)windowWidth / (float)windowHeight,
		0.1f,
		100.0f
	);

	modelShader.Use();

	modelShader.SetMat4("model", model);
	modelShader.SetMat4("view", view);
	modelShader.SetMat4("projection", projection);

	for (const Model& model : models) {
		model.Draw(modelShader);
	}

	ui.Update(fps, windowWidth, windowHeight);
	ui.Render();
}

void Game::processInput()
{
	if (glfwGetKey(window, GLFW_KEY_ESCAPE) == GLFW_PRESS)
	{
		glfwSetWindowShouldClose(window, true);
		gameState = GAME_END;
	}

	if (glfwGetKey(window, GLFW_KEY_W) == GLFW_PRESS)
		camera.ProcessKeyboard(FORWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_S) == GLFW_PRESS)
		camera.ProcessKeyboard(BACKWARD, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_A) == GLFW_PRESS)
		camera.ProcessKeyboard(LEFT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_D) == GLFW_PRESS)
		camera.ProcessKeyboard(RIGHT, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_SPACE) == GLFW_PRESS)
		camera.ProcessKeyboard(UP, deltaTime);

	if (glfwGetKey(window, GLFW_KEY_LEFT_SHIFT) == GLFW_PRESS)
		camera.ProcessKeyboard(DOWN, deltaTime);

	bool tabPressed = glfwGetKey(window, GLFW_KEY_TAB) == GLFW_PRESS;

	if (tabPressed && !tabWasPressed)
	{
		setCursorLocked(!cursorLocked);
	}

	tabWasPressed = tabPressed;
}

void Game::setCursorLocked(bool locked)
{
	cursorLocked = locked;

	glfwSetInputMode(
		window,
		GLFW_CURSOR,
		locked ? GLFW_CURSOR_DISABLED : GLFW_CURSOR_NORMAL
	);
}

void framebuffer_size_callback(GLFWwindow* window, int width, int height)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

	if (game)
		game->OnFramebufferResize(width, height);
}

void mouse_callback(GLFWwindow* window, double xpos, double ypos)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

	if (game)
		game->OnMouseMove(xpos, ypos);
}

void scroll_callback(GLFWwindow* window, double xoffset, double yoffset)
{
	Game* game = static_cast<Game*>(glfwGetWindowUserPointer(window));

	if (game)
		game->OnMouseScroll(xoffset, yoffset);
}

void Game::OnFramebufferResize(int width, int height)
{
	windowWidth = width;
	windowHeight = height;

	glViewport(0, 0, width, height);
}

void Game::OnMouseMove(double xposIn, double yposIn)
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

	camera.ProcessMouseMovement(xoffset, yoffset);
}

void Game::OnMouseScroll(double xoffset, double yoffset)
{
	camera.ProcessMouseScroll(static_cast<float>(yoffset));
}