#pragma once
#include "Graphics/Shader.hpp";

class Sky {
public:
	Sky();
	~Sky();

	void SetUp();

	void Render(const glm::mat4& invViewProjection, const glm::vec2& screenSize);


private:
	unsigned int VAO, VBO;
	Shader shader;
};