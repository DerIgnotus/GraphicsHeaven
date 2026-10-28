#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>

#include "Graphics/Shader.hpp"

class Water
{
public:
	explicit Water(float seaLevel = 0.0f);

	Water(const Water&) = delete;
	Water& operator=(const Water&) = delete;

	void BeginDraw(const glm::mat4& view, const glm::mat4& projection, const glm::vec3& cameraPos);
	Shader& GetShader();

	float GetSeaLevel() const { return seaLevel; }

private:
	float seaLevel;
	Shader shader;
};
