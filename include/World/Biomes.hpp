#pragma once

enum class Biome
{
	Ocean,
	Plains,
	Desert,
	Forest,
	Hills,
	Mountains,
	Snow
};

struct BiomeData
{
	float continentalness;
	float temperature;
	float moisture;
};

inline Biome GetBiome(const BiomeData& data)
{
	//std::cout << "continent: " << data.continentalness << " | temeprature: " << data.temperature << " | moisture: " << data.moisture << std::endl;
	if (data.continentalness < 0.30f)
	{
		//std::cout << "Ocean" << std::endl;

		return Biome::Ocean;
	}
	if (data.temperature < 0.20f)
	{
		//std::cout << "Snow" << std::endl;

		return Biome::Snow;
	}

	if (data.temperature > 0.65f && data.moisture < 0.30f)
	{
		//std::cout << "Desert" << std::endl;

		return Biome::Desert;
	}

	if (data.moisture > 0.60f)
	{
		//std::cout << "Forest" << std::endl;

		return Biome::Forest;
	}

	if (data.continentalness > 0.65f)
	{
		//std::cout << "Mountains" << std::endl;

		return Biome::Mountains;
	}

	//std::cout << "Plains" << std::endl;

	return Biome::Plains;
}