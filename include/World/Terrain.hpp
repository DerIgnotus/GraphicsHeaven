#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Graphics/Shader.hpp"
#include "Utility/Noise.hpp"
#include <Graphics/Mesh.hpp>

static const float scale = 0.35f;
static const int octaves = 4;
static const float persistance = 0.5f;
static const float lacunarity = 2.0f;

struct MeshData {
	int chunkX, chunkZ;
	int lod;

	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class Terrain {
public:
	Terrain();
	~Terrain();

	static MeshData GenerateData(int chunkX, int chunkZ, int seed, float heightMultiplier, int meshStep);

	void UploadToGPU(const MeshData& data);

	void Draw(glm::mat4& model, Shader& shader);

private:
	const int mapChunkSize = 241;

	size_t indexCount = 0;

	unsigned int VAO, VBO, EBO;
};