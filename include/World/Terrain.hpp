#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Graphics/Shader.hpp"
#include "Utility/Noise.hpp"
#include <Graphics/Mesh.hpp>
#include "Biomes.hpp"
#include "Ecosystem/Grass.hpp"

static const float scale = 0.35f;
static const int octaves = 4;
static const float persistance = 0.5f;
static const float lacunarity = 2.0f;

struct MeshData
{
	int chunkX, chunkZ;
	int lod;

	std::vector<DebugVertex> vertices;
	//std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;

	//std::vector<glm::vec3> waterVertices;
	//std::vector<unsigned int> waterIndices;

	std::vector<GrassInstance> grassInstances;
};


class Terrain {
public:
	Terrain();
	~Terrain();

	static MeshData GenerateData(
		int chunkX, int chunkZ, int seed,
		float heightMultiplier, int meshStep
	);

	void UploadToGPU(const MeshData& data);

	void Draw(glm::mat4& model, Shader& shader);
	void DrawWater(glm::mat4& model, Shader& shader);

private:
	const int mapChunkSize = 241;

	size_t indexCount = 0;
	size_t waterIndexCount = 0;

	unsigned int VAO = 0, VBO = 0, EBO = 0;
	unsigned int waterVAO = 0, waterVBO = 0, waterEBO = 0;
};
