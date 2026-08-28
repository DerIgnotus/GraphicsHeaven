#pragma once

#include "Shader.hpp"
#include "Texture.hpp"

#include <glad/glad.h>
#include <glm/glm.hpp>

#include <vector>
#include <memory>

struct Vertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec2 texCoord;
};

struct Material
{
	glm::vec4 baseColor = glm::vec4(1.0f);

	float metallic = 0.0f;
	float roughness = 1.0f;

	glm::vec3 emissive = glm::vec3(0.0f);

	std::shared_ptr<Texture> baseColorTexture;
	std::shared_ptr<Texture> metallicRoughnessTexture;
	std::shared_ptr<Texture> normalTexture;
	std::shared_ptr<Texture> occlusionTexture;
	std::shared_ptr<Texture> emissiveTexture;
};

class Mesh
{
public:
	Mesh(const std::vector<Vertex>& vertices, const std::vector<unsigned int>& indices, const Material& material);
	~Mesh();

	Mesh(const Mesh&) = delete;
	Mesh& operator=(const Mesh&) = delete;

	Mesh(Mesh&& other) noexcept;
	Mesh& operator=(Mesh&& other) noexcept;

	void Draw(Shader& shader, unsigned int skyboxTexture) const;

private:
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	Material material;

	unsigned int VAO = 0;
	unsigned int VBO = 0;
	unsigned int EBO = 0;

	void setupMesh();
};