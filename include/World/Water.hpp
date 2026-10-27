#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Graphics/Shader.hpp"

class Water
{
public:
	explicit Water(float seaLevel = 25.0f);
	~Water();

	Water(const Water&) = delete;
	Water& operator=(const Water&) = delete;

	void Draw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos, float waterSize);

private:
	void InitializeMesh();

	float seaLevel;

	Shader shader;

	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;
};
