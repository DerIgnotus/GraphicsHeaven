#pragma once
#include <glm/ext/vector_float2.hpp>

class UI {
public:
	UI();
	~UI();

	void Update(double fps, glm::vec2 windowSize, float& timeMultiplier, float& camMovementSpeed);
	void Render();

private:


};