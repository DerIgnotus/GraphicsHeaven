#include "World/Water.hpp"
#include <GLFW/glfw3.h>

Water::Water(float seaLevel) :
	seaLevel(seaLevel),
	shader("shaders/water.vert", "shaders/water.frag")
{
}

void Water::BeginDraw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos)
{
	shader.Use();
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);
	shader.SetFloat("seaLevel", seaLevel);

	shader.SetVec3("cameraPos", cameraPos);
	shader.SetFloat("time", static_cast<float>(glfwGetTime()));
}

Shader& Water::GetShader()
{
	return shader;
}
