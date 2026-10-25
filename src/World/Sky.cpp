#include "World/Sky.hpp"

Sky::Sky() : VAO(0), VBO(0), noiseTextureID(0), sunDirection(0.0f, 1.0f, 0.0f) {

}

Sky::~Sky() {
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
}

void Sky::SetUp() {
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
	glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 2 * sizeof(float), (void*)0);
	glBindVertexArray(0);
}

void Sky::Update(float time) {
	// 24-hour cycle orbit calculation
	float sunAngle = (time / 24.0f) * 2.0f * 3.1415926535f;
	sunDirection = glm::normalize(glm::vec3(0.0f, sin(sunAngle), cos(sunAngle)));
}

void Sky::Render(const glm::mat4& invViewProjection, const glm::vec2& screenSize, const glm::vec3& cameraPosition, float time) {
	glDisable(GL_DEPTH_TEST);
	glDepthMask(GL_FALSE);

	shader.Use();
	shader.SetMat4("invViewProjection", invViewProjection);
	shader.SetVec2("screenSize", screenSize);
	shader.SetVec3("cameraPosition", cameraPosition);
	shader.SetVec3("sunDirection", sunDirection);
	shader.SetFloat("time", time);

	glBindVertexArray(VAO);
	glDrawArrays(GL_TRIANGLES, 0, 3);

	glDepthMask(GL_TRUE);
	glEnable(GL_DEPTH_TEST);
}