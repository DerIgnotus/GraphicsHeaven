#pragma once
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <random>

#include "FastNoiseLite.h"

namespace NoiseGen {
	inline std::vector<float> GenerateNoiseMap(int mapWidth, int mapHeight, float scale, int octaves, float persistance, float lacunarity, int seed, glm::vec2 worldOffset = glm::vec2(0, 0), FastNoiseLite::NoiseType noiseType = FastNoiseLite::NoiseType::NoiseType_Perlin)
	{
		FastNoiseLite noise;
		noise.SetSeed(seed);
		noise.SetNoiseType(noiseType);

		std::vector<float> noiseMap(mapWidth * mapHeight);

		if (scale <= 0.0f)
		{
			scale = 0.00001f;
		}

		for (int z = 0; z < mapHeight; z++)
		{
			for (int x = 0; x < mapWidth; x++)
			{
				float amplitude = 1.0f;
				float frequency = 1.0f;
				float noiseHeight = 0.0f;

				float sampleX = (worldOffset.x + x) * scale;
				float sampleZ = (worldOffset.y + z) * scale;

				for (int i = 0; i < octaves; i++)
				{
					float perlinValue = noise.GetNoise(sampleX * frequency, sampleZ * frequency);
					noiseHeight += perlinValue * amplitude;

					amplitude *= persistance;
					frequency *= lacunarity;
				}

				noiseMap[z * mapWidth + x] = noiseHeight;
			}
		}

		return noiseMap;
	}
}