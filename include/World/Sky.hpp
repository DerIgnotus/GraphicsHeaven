#pragma once

#include "Graphics/Shader.hpp";

class Sky {
public:
	Sky();
	~Sky();

	Sky(const Sky&) = delete;
	Sky& operator=(const Sky&) = delete;

	void SetUp();

	void Update(float time);
	void Render(const glm::mat4& invViewProjection, const glm::vec2& screenSize, const glm::vec3& cameraPosition, float time);

	glm::vec3 GetSunDirection() const { return sunDirection; }


private:
	unsigned int VAO, VBO;
	unsigned int noiseTextureID;
	Shader shader;

	glm::vec3 sunDirection = glm::vec3(0.0f, 1.0f, 0.0f);
};