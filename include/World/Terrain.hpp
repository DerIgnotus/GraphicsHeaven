#pragma once

#include <glm/glm.hpp>
#include <vector>

#include "Graphics/Shader.hpp"
#include "Utility/Noise.hpp"
#include <Graphics/Mesh.hpp>

struct MeshData {
	int chunkX, chunkZ;
	std::vector<Vertex> vertices;
	std::vector<unsigned int> indices;
};

class Terrain {
public:
	Terrain();
	~Terrain();

	static MeshData GenerateData(int chunkX, int chunkZ, int seed, float heightMultiplier, int meshStep);

	void UploadToGPU(const MeshData& data);

	void Draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection, Shader& shader);

private:
	const int mapChunkSize = 241;

	size_t indexCount = 0;

	unsigned int VAO, VBO, EBO;
};