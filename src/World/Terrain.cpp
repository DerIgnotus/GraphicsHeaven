#include "World/Terrain.hpp"

float Smooth01(float t)
{
	t = glm::clamp(t, 0.0f, 1.0f);
	return t * t * (3.0f - 2.0f * t);
}

float GetBlendedBiomeHeight(
	const BiomeData& biomeData,
	float baseNoise,
	float detailNoise,
	float heightMultiplier
)
{
	float inland = Smooth01(
		(biomeData.continentalness - 0.30f) / 0.35f
	);

	float mountainFactor = Smooth01(
		(biomeData.continentalness - 0.55f) / 0.25f
	);

	float desertFactor =
		Smooth01((biomeData.temperature - 0.45f) / 0.30f) *
		(1.0f - Smooth01(
			(biomeData.moisture - 0.25f) / 0.30f
		));

	float plainsHeight =
		baseNoise * 8.0f + detailNoise * 2.0f;

	float desertHeight =
		baseNoise * 10.0f + std::abs(detailNoise) * 5.0f;

	float mountainHeight =
		baseNoise * 65.0f + detailNoise * 12.0f;

	float height = glm::mix(
		plainsHeight,
		desertHeight,
		desertFactor
	);

	height = glm::mix(
		height,
		mountainHeight,
		mountainFactor
	);

	height = glm::mix(
		-25.0f + baseNoise * 5.0f,
		height,
		inland
	);

	return height * heightMultiplier;
}

Terrain::Terrain() : VAO(0), VBO(0), EBO(0), waterEBO(0), waterVBO(0), waterVAO(0)
{
}

Terrain::~Terrain()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);

	if (waterEBO != 0)
		glDeleteBuffers(1, &waterEBO);

	if (waterVBO != 0)
		glDeleteBuffers(1, &waterVBO);

	if (waterVAO != 0)
		glDeleteVertexArrays(1, &waterVAO);
}

MeshData Terrain::GenerateData(int chunkX, int chunkZ, int seed, float heightMultiplier, int meshStep)
{
	FastNoiseLite noise;
	noise.SetSeed(seed);
	noise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);

	// BIOMES

	FastNoiseLite continentalnessNoise;
	continentalnessNoise.SetSeed(seed + 101);
	continentalnessNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	//continentalnessNoise.SetFractalType(FastNoiseLite::FractalType_None);

	FastNoiseLite temperatureNoise;
	temperatureNoise.SetSeed(seed + 202);
	temperatureNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	//temperatureNoise.SetFractalType(FastNoiseLite::FractalType_None);

	FastNoiseLite moistureNoise;
	moistureNoise.SetSeed(seed + 303);
	moistureNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);
	//moistureNoise.SetFractalType(FastNoiseLite::FractalType_None);

	FastNoiseLite detailNoise;
	detailNoise.SetSeed(seed + 404);
	detailNoise.SetNoiseType(FastNoiseLite::NoiseType::NoiseType_Perlin);

	// END BIOMES

	if (meshStep < 1) meshStep = 1;
	const int mapChunkSize = 241;

	float worldOffsetX = chunkX * (mapChunkSize - 1);
	float worldOffsetZ = chunkZ * (mapChunkSize - 1);

	//const std::vector<float> noiseMap = NoiseGen::GenerateNoiseMap(
	//	noise,
	//	mapChunkSize, mapChunkSize,
	//	scale, octaves, persistance, lacunarity,
	//	glm::vec2(worldOffsetX, worldOffsetZ)
	//);

	auto getBiomeData = [&](float worldX, float worldZ) -> BiomeData {
		BiomeData data =
		{
			NoiseGen::GetNoise01(
				continentalnessNoise,
				worldX, worldZ, 0.044f
			),

			NoiseGen::GetNoise01(
				temperatureNoise,
				worldX, worldZ, 0.026f
			),

			NoiseGen::GetNoise01(
				moistureNoise,
				worldX, worldZ, 0.034f
			)
		};

		return data;
		};

	auto isOcean = [&](int x, int z) -> bool
		{
			const float worldX = static_cast<float>(worldOffsetX + x);
			const float worldZ = static_cast<float>(worldOffsetZ + z);

			return getBiomeData(worldX, worldZ).continentalness < 0.30f;
		};

	auto getTerrainHeight = [&](
		float worldX,
		float worldZ
		) -> float
		{
			BiomeData biomeData = getBiomeData(worldX, worldZ);

			float base = noise.GetNoise(
				worldX * 0.03f,
				worldZ * 0.03f
			);

			float detail = detailNoise.GetNoise(
				worldX * 0.2f,
				worldZ * 0.2f
			);

			return GetBlendedBiomeHeight(
				biomeData,
				base,
				detail,
				heightMultiplier
			);
		};

	MeshData data;
	data.chunkX = chunkX;
	data.chunkZ = chunkZ;
	data.lod = meshStep;

	int vertsPerRow = (mapChunkSize - 1) / meshStep + 1;
	data.vertices.reserve(vertsPerRow * vertsPerRow + (vertsPerRow * 4));

	const float halfSize = (mapChunkSize - 1) / 2.0f;

	for (int z = 0; z < mapChunkSize; z += meshStep)
	{
		for (int x = 0; x < mapChunkSize; x += meshStep)
		{
			DebugVertex vertex;

			float worldX = worldOffsetX + x;
			float worldZ = worldOffsetZ + z;

			float height = getTerrainHeight(worldX, worldZ);

			vertex.position = glm::vec3(
				static_cast<float>(x) - halfSize,
				height,
				static_cast<float>(z) - halfSize
			);

			float left = getTerrainHeight(worldX - 1.0f, worldZ);
			float right = getTerrainHeight(worldX + 1.0f, worldZ);
			float down = getTerrainHeight(worldX, worldZ - 1.0f);
			float up = getTerrainHeight(worldX, worldZ + 1.0f);

			vertex.normal = glm::normalize(glm::vec3(
				left - right,
				2.0f,
				down - up
			));

			vertex.texCoord = glm::vec2(
				static_cast<float>(x) / (mapChunkSize - 1),
				glm::clamp((height + 100.0f) / 200.0f, 0.0f, 1.0f)
			);

			BiomeData biomeData = getBiomeData(worldX, worldZ);

			//std::cout << "Cont: " << biomeData.continentalness << " | Temp: " << biomeData.temperature << " | Moist: " << biomeData.moisture << std::endl;

			vertex.continentalness = biomeData.continentalness;
			vertex.temperature = biomeData.temperature;
			vertex.moisture = biomeData.moisture;

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

	float skirtDepth = 30.0f;

	auto addSkirtQuad = [&](unsigned int v1Idx, unsigned int v2Idx)
		{
			DebugVertex v1 = data.vertices[v1Idx];
			DebugVertex v2 = data.vertices[v2Idx];

			v1.position.y -= skirtDepth;
			v2.position.y -= skirtDepth;

			unsigned int skirt1Idx = static_cast<unsigned int>(data.vertices.size());
			data.vertices.push_back(v1);

			unsigned int skirt2Idx = static_cast<unsigned int>(data.vertices.size());
			data.vertices.push_back(v2);

			data.indices.push_back(v1Idx);
			data.indices.push_back(v2Idx);
			data.indices.push_back(skirt1Idx);

			data.indices.push_back(skirt1Idx);
			data.indices.push_back(v2Idx);
			data.indices.push_back(skirt2Idx);
		};


	// South Edge (z = 0)
	for (int x = 0; x < vertsPerRow - 1; ++x)
	{
		addSkirtQuad(0 * vertsPerRow + x, 0 * vertsPerRow + (x + 1));
	}

	// North Edge (z = vertsPerRow - 1)
	for (int x = 0; x < vertsPerRow - 1; ++x)
	{
		int z = vertsPerRow - 1;
		addSkirtQuad(z * vertsPerRow + (x + 1), z * vertsPerRow + x);
	}

	// West Edge (x = 0)
	for (int z = 0; z < vertsPerRow - 1; ++z)
	{
		addSkirtQuad((z + 1) * vertsPerRow + 0, z * vertsPerRow + 0);
	}

	// East Edge (x = vertsPerRow - 1)
	for (int z = 0; z < vertsPerRow - 1; ++z)
	{
		int x = vertsPerRow - 1;
		addSkirtQuad(z * vertsPerRow + x, (z + 1) * vertsPerRow + x);
	}

	const int samplesPerSide = (mapChunkSize - 1) / meshStep + 1;

	for (int z = 0; z < samplesPerSide - 1; ++z)
	{
		for (int x = 0; x < samplesPerSide - 1; ++x)
		{
			const int x0 = x * meshStep;
			const int z0 = z * meshStep;
			const int x1 = x0 + meshStep;
			const int z1 = z0 + meshStep;

			if (!isOcean(x0, z0) ||
				!isOcean(x1, z0) ||
				!isOcean(x0, z1) ||
				!isOcean(x1, z1))
			{
				continue;
			}

			const unsigned int base =
				static_cast<unsigned int>(data.waterVertices.size());

			data.waterVertices.emplace_back(
				x0 - halfSize, 0.0f, z0 - halfSize);
			data.waterVertices.emplace_back(
				x1 - halfSize, 0.0f, z0 - halfSize);
			data.waterVertices.emplace_back(
				x1 - halfSize, 0.0f, z1 - halfSize);
			data.waterVertices.emplace_back(
				x0 - halfSize, 0.0f, z1 - halfSize);

			data.waterIndices.push_back(base + 0);
			data.waterIndices.push_back(base + 1);
			data.waterIndices.push_back(base + 2);

			data.waterIndices.push_back(base + 0);
			data.waterIndices.push_back(base + 2);
			data.waterIndices.push_back(base + 3);
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
	glBufferData(GL_ARRAY_BUFFER, data.vertices.size() * sizeof(DebugVertex), data.vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, data.indices.size() * sizeof(unsigned int), data.indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, texCoord));
	glEnableVertexAttribArray(2);

	glVertexAttribPointer(3, 1, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, continentalness));
	glEnableVertexAttribArray(3);

	glVertexAttribPointer(4, 1, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, temperature));
	glEnableVertexAttribArray(4);

	glVertexAttribPointer(5, 1, GL_FLOAT, GL_FALSE, sizeof(DebugVertex), (void*)offsetof(DebugVertex, moisture));
	glEnableVertexAttribArray(5);

	glBindVertexArray(0);


	if (waterVAO == 0)
	{
		glGenVertexArrays(1, &waterVAO);
		glGenBuffers(1, &waterVBO);
		glGenBuffers(1, &waterEBO);
	}

	waterIndexCount = data.waterIndices.size();

	glBindVertexArray(waterVAO);

	glBindBuffer(GL_ARRAY_BUFFER, waterVBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		data.waterVertices.size() * sizeof(glm::vec3),
		data.waterVertices.empty() ? nullptr : data.waterVertices.data(),
		GL_STATIC_DRAW
	);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, waterEBO);
	glBufferData(
		GL_ELEMENT_ARRAY_BUFFER,
		data.waterIndices.size() * sizeof(unsigned int),
		data.waterIndices.empty() ? nullptr : data.waterIndices.data(),
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0, 3, GL_FLOAT, GL_FALSE,
		sizeof(glm::vec3), (void*)0
	);

	glBindVertexArray(0);
}

void Terrain::DrawWater(glm::mat4& model, Shader& shader)
{
	if (waterVAO == 0 || waterIndexCount == 0)
		return;

	shader.SetMat4("model", model);

	glBindVertexArray(waterVAO);
	glDrawElements(GL_TRIANGLES, static_cast<GLsizei>(waterIndexCount), GL_UNSIGNED_INT, nullptr);
	glBindVertexArray(0);
}

void Terrain::Draw(glm::mat4& model, Shader& shader)
{
	if (indexCount == 0) return;

	shader.Use();

	shader.SetMat4("model", model);

	glBindVertexArray(VAO);

	glDrawElements(GL_TRIANGLES, indexCount, GL_UNSIGNED_INT, 0);

	glBindVertexArray(0);
}

