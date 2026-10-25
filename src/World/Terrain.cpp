#include "World/Terrain.hpp"

Terrain::Terrain() : VAO(0), VBO(0), EBO(0)
{
}

Terrain::~Terrain()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}

MeshData Terrain::GenerateData(int chunkX, int chunkZ, int seed, float heightMultiplier, int meshStep)
{
	if (meshStep < 1) meshStep = 1;
	const int mapChunkSize = 241;

	float worldOffsetX = chunkX * (mapChunkSize - 1);
	float worldOffsetZ = chunkZ * (mapChunkSize - 1);

	// Heavy CPU work happens safely here on a background thread
	const std::vector<float> noiseMap = NoiseGen::GenerateNoiseMap(
		mapChunkSize, mapChunkSize,
		0.5f, 3, 0.35f, 4.0f,
		seed,
		glm::vec2(worldOffsetX, worldOffsetZ)
	);

	MeshData data;
	data.chunkX = chunkX;
	data.chunkZ = chunkZ;

	int vertsPerRow = (mapChunkSize - 1) / meshStep + 1;
	data.vertices.reserve(vertsPerRow * vertsPerRow);

	for (int z = 0; z < mapChunkSize; z += meshStep)
	{
		for (int x = 0; x < mapChunkSize; x += meshStep)
		{
			Vertex vertex;
			float rawHeight = noiseMap[z * mapChunkSize + x];
			float height = rawHeight * heightMultiplier;

			vertex.position = glm::vec3((float)x - (mapChunkSize / 2.0f), height, (float)z - (mapChunkSize / 2.0f));
			vertex.normal = glm::vec3(0.0f, 1.0f, 0.0f);

			float normalizedHeight = (rawHeight + 1.0f) * 0.5f;
			vertex.texCoord = glm::vec2((float)x / mapChunkSize, normalizedHeight);

			data.vertices.push_back(vertex);
		}
	}

	for (int z = 0; z < vertsPerRow - 1; z++)
	{
		for (int x = 0; x < vertsPerRow - 1; x++)
		{
			unsigned int i0 = z * vertsPerRow + x;
			unsigned int i1 = (z + 1) * vertsPerRow + x;
			unsigned int i2 = z * vertsPerRow + (x + 1);
			unsigned int i3 = (z + 1) * vertsPerRow + (x + 1);

			data.indices.push_back(i0);
			data.indices.push_back(i1);
			data.indices.push_back(i2);

			data.indices.push_back(i2);
			data.indices.push_back(i1);
			data.indices.push_back(i3);
		}
	}

	return data;
}

void Terrain::UploadToGPU(const MeshData& data)
{
	indexCount = data.indices.size();

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(Vertex), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}

void Terrain::Draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection, Shader& shader)
{
	if (indexCount == 0) return;

	shader.Use();

	shader.SetMat4("model", model);
	shader.SetMat4("view", view);
	shader.SetMat4("projection", projection);

	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}
