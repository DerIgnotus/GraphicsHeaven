#include "World/Water.hpp"

#include <cmath>
#include <glm/ext/matrix_transform.hpp>
#include <GLFW/glfw3.h>

Water::Water(float seaLevel) :
	seaLevel(seaLevel),
	shader("shaders/water.vert", "shaders/water.frag")
{
	InitializeMesh();
}

Water::~Water()
{
	glDeleteBuffers(1, &EBO);
	glDeleteBuffers(1, &VBO);
	glDeleteVertexArrays(1, &VAO);
}

void Water::InitializeMesh()
{
	const float vertices[] = {
		-0.5f, 0.0f, -0.5f,
		 0.5f, 0.0f, -0.5f,
		 0.5f, 0.0f,  0.5f,
		-0.5f, 0.0f,  0.5f
	};

	const unsigned int indices[] = {
		0, 2, 1,
		0, 3, 2
	};

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vertices),
		vertices,
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		sizeof(indices),
		indices,
		GL_STATIC_DRAW
	);

	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		3 * sizeof(float), (void*)0
	);
	glEnableVertexAttribArray(0);

	glBindVertexArray(0);
}

void Water::Draw(
	const glm::mat4& view,
	const glm::mat4& projection,
	const glm::vec3& cameraPos,
	float waterSize)
{
	shader.Use();

	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);

	glm::mat4 model(1.0f);

	model = glm::translate(model, glm::vec3(cameraPos.x, seaLevel, cameraPos.z));

	model = glm::scale(model, glm::vec3(waterSize, 1.0f, waterSize));

	shader.SetMat4("model", model);

	shader.SetFloat("time", static_cast<float>(glfwGetTime()));

	glBindVertexArray(VAO);
	glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}
