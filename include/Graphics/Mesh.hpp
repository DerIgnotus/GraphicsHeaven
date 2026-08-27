#pragma once

#include "Shader.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices);
	~Mesh();

	void Draw(Shader& shader) const;

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	void setupMesh();
};