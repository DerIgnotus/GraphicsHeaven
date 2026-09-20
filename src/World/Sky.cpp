#include "World/Sky.hpp"

Sky::Sky()
{

}

Sky::~Sky()
{
}

void Sky::SetUp()
{
	float vertices[] = {
	-1.0f, -1.0f,
	 3.0f, -1.0f,
	-1.0f,  3.0f
	};

	shader = Shader("shaders/sky.vert", "shaders/sky.frag");

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(vertices),
		vertices,
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		2 * sizeof(float),
		(void*)0
	);

	glBindVertexArray(0);
}

void Sky::Render(const glm::mat4& invViewProjection, const glm::vec2& screenSize)
{
	glDisable(GL_DEPTH_TEST);

	shader.Use();
	shader.SetMat4("invViewProjection", invViewProjection);
	shader.SetVec2("screenSize", screenSize);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glEnable(GL_DEPTH_TEST);
}
