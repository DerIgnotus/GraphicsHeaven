
#include "Ecosystem/Grass.hpp"

#include <cmath>
#include <random>
#include <glm/gtc/constants.hpp>
#include <algorithm>
#include <cstddef>
#include <cstdint>
#include <utility>
#include <glm/ext/matrix_transform.hpp>

GrassMesh GenerateGrassClump()
{
	GrassMesh mesh;

	constexpr float width = 0.6f;
	constexpr float height = 1.0f;

	const glm::vec3 color(1.0f);

	mesh.vertices = {
		{
			glm::vec3(-width, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			color,
			glm::vec2(0.0f, 0.0f)
		},
		{
			glm::vec3(width, 0.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			color,
			glm::vec2(1.0f, 0.0f)
		},
		{
			glm::vec3(-width, height, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			color,
			glm::vec2(0.0f, 1.0f)
		},
		{
			glm::vec3(width, height, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			color,
			glm::vec2(1.0f, 1.0f)
		}
	};

	mesh.indices = {
		0, 1, 2,
		1, 3, 2
	};

	return mesh;
}


GrassRenderer::ChunkGrass* GrassRenderer::FindChunk(
	int chunkX, int chunkZ)
{
	for (auto& entry : chunkGrass)
	{
		if (entry.first.x == chunkX && entry.first.y == chunkZ)
			return &entry.second;
	}

	return nullptr;
}


GrassRenderer::~GrassRenderer()
{
	Cleanup();
}


void GrassRenderer::Initialize()
{
	if (initialized)
		return;

	const GrassMesh mesh = GenerateGrassClump();

	if (mesh.vertices.empty() || mesh.indices.empty())
		return;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(
			mesh.vertices.size() * sizeof(GrassVertex)),
		mesh.vertices.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(
			mesh.indices.size() * sizeof(unsigned int)),
		mesh.indices.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
		reinterpret_cast<void*>(offsetof(GrassVertex, position))
	);

	glEnableVertexAttribArray(1);
	glVertexAttribPointer(
		1, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
		reinterpret_cast<void*>(offsetof(GrassVertex, normal))
	);

	glEnableVertexAttribArray(2);
	glVertexAttribPointer(
		2, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
		reinterpret_cast<void*>(offsetof(GrassVertex, color))
	);

	glEnableVertexAttribArray(3);
	glVertexAttribPointer(
		3, 2, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
		reinterpret_cast<void*>(offsetof(GrassVertex, texCoords))
	);

	glBindVertexArray(0);

	indexCount = static_cast<GLsizei>(mesh.indices.size());
	initialized = true;
}


void GrassRenderer::UploadInstances(
	int chunkX,
	int chunkZ,
	const std::vector<GrassInstance>& instances)
{
	if (!initialized)
		Initialize();

	if (!initialized)
		return;

	ChunkGrass* chunk = FindChunk(chunkX, chunkZ);

	if (chunk == nullptr)
	{
		chunkGrass.emplace_back(
			glm::ivec2(chunkX, chunkZ),
			ChunkGrass{}
		);

		chunk = &chunkGrass.back().second;

		glGenVertexArrays(1, &chunk->VAO);
		glGenBuffers(1, &chunk->instanceVBO);

		glBindVertexArray(chunk->VAO);

		glBindBuffer(GL_ARRAY_BUFFER, VBO);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(
			0, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
			reinterpret_cast<void*>(offsetof(GrassVertex, position))
		);

		glEnableVertexAttribArray(1);
		glVertexAttribPointer(
			1, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
			reinterpret_cast<void*>(offsetof(GrassVertex, normal))
		);

		glEnableVertexAttribArray(2);
		glVertexAttribPointer(
			2, 3, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
			reinterpret_cast<void*>(offsetof(GrassVertex, color))
		);

		glEnableVertexAttribArray(3);
		glVertexAttribPointer(
			3, 2, GL_FLOAT, GL_FALSE, sizeof(GrassVertex),
			reinterpret_cast<void*>(offsetof(GrassVertex, texCoords))
		);

		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);

		glBindBuffer(GL_ARRAY_BUFFER, chunk->instanceVBO);

		constexpr GLsizei stride = sizeof(GPUInstance);

		glEnableVertexAttribArray(4);
		glVertexAttribPointer(
			4, 3, GL_FLOAT, GL_FALSE, stride,
			reinterpret_cast<void*>(offsetof(GPUInstance, position))
		);
		glVertexAttribDivisor(4, 1);

		glEnableVertexAttribArray(5);
		glVertexAttribPointer(
			5, 1, GL_FLOAT, GL_FALSE, stride,
			reinterpret_cast<void*>(offsetof(GPUInstance, rotation))
		);
		glVertexAttribDivisor(5, 1);

		glEnableVertexAttribArray(6);
		glVertexAttribPointer(
			6, 1, GL_FLOAT, GL_FALSE, stride,
			reinterpret_cast<void*>(offsetof(GPUInstance, scale))
		);
		glVertexAttribDivisor(6, 1);

		glEnableVertexAttribArray(7);
		glVertexAttribPointer(
			7, 1, GL_FLOAT, GL_FALSE, stride,
			reinterpret_cast<void*>(offsetof(GPUInstance, colorVariation))
		);
		glVertexAttribDivisor(7, 1);

		glBindVertexArray(0);
	}

	chunk->instanceCount = static_cast<GLsizei>(instances.size());

	if (instances.empty())
	{
		glBindBuffer(GL_ARRAY_BUFFER, chunk->instanceVBO);
		glBufferData(GL_ARRAY_BUFFER, 0, nullptr, GL_STATIC_DRAW);
		glBindBuffer(GL_ARRAY_BUFFER, 0);
		return;
	}

	std::vector<GPUInstance> gpuInstances;
	gpuInstances.reserve(instances.size());

	for (const GrassInstance& instance : instances)
	{
		gpuInstances.push_back({
			instance.position,
			instance.rotation,
			instance.scale,
			instance.colorVariation
			});
	}

	glBindBuffer(GL_ARRAY_BUFFER, chunk->instanceVBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		static_cast<GLsizeiptr>(
			gpuInstances.size() * sizeof(GPUInstance)),
		gpuInstances.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ARRAY_BUFFER, 0);
}


void GrassRenderer::Draw(
	int chunkX,
	int chunkZ,
	const glm::mat4& model,
	const glm::mat4& view,
	const glm::mat4& projection,
	Shader& shader)
{
	if (!initialized || indexCount == 0)
		return;

	ChunkGrass* chunk = FindChunk(chunkX, chunkZ);

	if (chunk == nullptr || chunk->instanceCount == 0)
		return;

	glm::mat4 drawModel = model;
	drawModel = glm::translate(drawModel, glm::vec3(0.0, -0.1f, 0.0));

	shader.Use();
	shader.SetMat4("model", model);
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);

	glBindVertexArray(chunk->VAO);

	glDrawElementsInstanced(
		GL_TRIANGLES,
		indexCount,
		GL_UNSIGNED_INT,
		nullptr,
		chunk->instanceCount
	);

	glBindVertexArray(0);
}


void GrassRenderer::RemoveChunk(int chunkX, int chunkZ)
{
	for (auto it = chunkGrass.begin(); it != chunkGrass.end(); ++it)
	{
		if (it->first.x == chunkX && it->first.y == chunkZ)
		{
			if (it->second.instanceVBO != 0)
				glDeleteBuffers(1, &it->second.instanceVBO);

			if (it->second.VAO != 0)
				glDeleteVertexArrays(1, &it->second.VAO);

			chunkGrass.erase(it);
			return;
		}
	}
}


void GrassRenderer::Cleanup()
{
	for (auto& entry : chunkGrass)
	{
		if (entry.second.instanceVBO != 0)
			glDeleteBuffers(1, &entry.second.instanceVBO);

		if (entry.second.VAO != 0)
			glDeleteVertexArrays(1, &entry.second.VAO);
	}

	chunkGrass.clear();

	if (EBO != 0)
		glDeleteBuffers(1, &EBO);

	if (VBO != 0)
		glDeleteBuffers(1, &VBO);

	if (VAO != 0)
		glDeleteVertexArrays(1, &VAO);

	VAO = VBO = EBO = 0;
	indexCount = 0;
	initialized = false;
}
