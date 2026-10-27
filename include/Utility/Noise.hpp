#pragma once
#include <vector>
#include <limits>
#include <glm/glm.hpp>
#include <random>

#include "FastNoiseLite.h"

namespace NoiseGen {
	inline std::vector<float> GenerateNoiseMap(
		FastNoiseLite& noise, int mapWidth, int mapHeight, float scale, int octaves,
		float persistance, float lacunarity, glm::vec2 worldOffset = glm::vec2(0, 0)
	)
	{
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

	inline float GetHeight(FastNoiseLite& noise, float worldX, float worldZ, float scale, int octaves, float persistance, float lacunarity)
	{
		if (scale <= 0.0f)
			scale = 0.00001f;

		float amplitude = 1.0f;
		float frequency = 1.0f;
		float noiseHeight = 0.0f;

		float sampleX = worldX * scale;
		float sampleZ = worldZ * scale;

		for (int i = 0; i < octaves; i++)
		{
			float perlinValue = noise.GetNoise(
				sampleX * frequency,
				sampleZ * frequency
			);

			noiseHeight += perlinValue * amplitude;

			amplitude *= persistance;
			frequency *= lacunarity;
		}

		return noiseHeight;
	}
}

