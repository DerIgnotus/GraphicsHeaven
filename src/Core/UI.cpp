#include "Core/UI.hpp"
#include "imgui.h"
#include "imgui_impl_glfw.h"
#include "imgui_impl_opengl3.h"

UI::UI()
{
}

UI::~UI()
{
}

void UI::Update(double fps, unsigned int windowWidth, unsigned int windowHeight)
{
	ImGui_ImplOpenGL3_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();

	ImGui::Begin("Information");

	ImGui::Text("FPS: %.1f", fps);
	ImGui::Text("Window: %u x %u", windowWidth, windowHeight);


	ImGui::End();
}

void UI::Render()
{
	ImGui::Render();
	ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());
}


