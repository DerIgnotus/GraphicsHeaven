#pragma once

#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>

#include "Graphics/Shader.hpp"

struct GrassVertex
{
	glm::vec3 position;
	glm::vec3 normal;
	glm::vec3 color;
	glm::vec2 texCoords;
};

struct GrassMesh
{
	std::vector<GrassVertex> vertices;
	std::vector<unsigned int> indices;
};

struct GrassInstance
{
	glm::vec3 position;
	float rotation;
	float scale;
	float colorVariation;
};

GrassMesh GenerateGrassClump();

class GrassRenderer
{
public:
	GrassRenderer() = default;
	~GrassRenderer();

	GrassRenderer(const GrassRenderer&) = delete;
	GrassRenderer& operator=(const GrassRenderer&) = delete;

	void Initialize();

	void UploadInstances(int chunkX, int chunkZ, const std::vector<GrassInstance>& instances);

	void Draw(int chunkX,
		int chunkZ,
		const glm::mat4& model,
		const glm::mat4& view,
		const glm::mat4& projection,
		Shader& shader);

	void RemoveChunk(int chunkX, int chunkZ);

	void Cleanup();

private:
	struct ChunkGrass
	{
		GLuint VAO = 0;
		GLuint instanceVBO = 0;
		GLsizei instanceCount = 0;
	};

	struct GPUInstance
	{
		glm::vec3 position;
		float rotation;
		float scale;
		float colorVariation;
	};

	GLuint VAO = 0;
	GLuint VBO = 0;
	GLuint EBO = 0;

	GLsizei indexCount = 0;
	bool initialized = false;

	std::vector<std::pair<glm::ivec2, ChunkGrass>> chunkGrass;

	ChunkGrass* FindChunk(int chunkX, int chunkZ);
};