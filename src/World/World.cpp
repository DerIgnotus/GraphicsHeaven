#include "World/World.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>

//World::World() {}
World::~World() {}

World::World(int seed, float heightMultiplier, int lod) :
	seed(seed), heightMultiplier(heightMultiplier),
	lod(lod), terrainShader("shaders/terrain.vert", "shaders/terrain.frag"),
	desertTexture("assets/textures/sand_color.jpg", TextureType::Color),
	plainsTexture("assets/textures/grass_color.jpg", TextureType::Color),
	mountainsTexture("assets/textures/rock_color.jpg", TextureType::Color),
	snowTexture("assets/textures/snow_color.jpg", TextureType::Color),
	hillsTexture("assets/textures/hill_color.jpg", TextureType::Color),
	forestTexture("assets/textures/forest_color.jpg", TextureType::Color)
{
	terrainShader.Use();

	terrainShader.SetInt("plainsTexture", 0);
	terrainShader.SetInt("desertTexture", 1);
	terrainShader.SetInt("forestTexture", 2);
	terrainShader.SetInt("hillsTexture", 3);
	terrainShader.SetInt("mountainsTexture", 4);
	terrainShader.SetInt("snowTexture", 5);

	terrainShader.SetFloat("textureScale", 0.05f);

	terrainShader.SetFloat("waterLevel", waterLevel);
}

void World::Update(const glm::vec3& cameraPos)
{
	backgroundTasks.erase(
		std::remove_if(backgroundTasks.begin(), backgroundTasks.end(),
			[](const std::future<void>& f) {
				return f.wait_for(std::chrono::seconds(0)) == std::future_status::ready;
			}),
		backgroundTasks.end()
	);

	{
		std::lock_guard<std::mutex> lock(queueMutex);
		if (!completedChunksQueue.empty())
		{
			MeshData data = std::move(completedChunksQueue.front());
			completedChunksQueue.pop();

			std::pair<int, int> key = { data.chunkX, data.chunkZ };
			loadingChunks.erase(key);

			auto terrain = std::make_unique<Terrain>();
			terrain->UploadToGPU(data);

			activeChunks[key] = std::move(terrain);
			activeChunkLODs[key] = data.lod;
		}
	}

	int playerChunkX = static_cast<int>(std::floor((cameraPos.x + (chunkSize / 2.0f)) / chunkSize));
	int playerChunkZ = static_cast<int>(std::floor((cameraPos.z + (chunkSize / 2.0f)) / chunkSize));

	std::set<std::pair<int, int>> requiredChunks;
	for (int xOffset = -renderDistance; xOffset <= renderDistance; xOffset++)
	{
		for (int zOffset = -renderDistance; zOffset <= renderDistance; zOffset++)
		{
			requiredChunks.insert({ playerChunkX + xOffset, playerChunkZ + zOffset });
		}
	}

	for (auto it = activeChunks.begin(); it != activeChunks.end();)
	{
		const auto& key = it->first;

		if (requiredChunks.find(key) == requiredChunks.end())
		{
			activeChunkLODs.erase(key);
			it = activeChunks.erase(it);
		}
		else
		{
			++it;
		}
	}

	for (const auto& key : requiredChunks)
	{
		int targetLOD = CalculateLOD(key.first, key.second, playerChunkX, playerChunkZ);

		bool needsGeneration =
			activeChunks.find(key) == activeChunks.end() ||
			activeChunkLODs[key] != targetLOD;

		if (needsGeneration && loadingChunks.find(key) == loadingChunks.end())
		{
			loadingChunks.insert(key);

			int targetX = key.first;
			int targetZ = key.second;
			int s = seed;
			float hm = heightMultiplier;

			backgroundTasks.push_back(std::async(
				std::launch::async,
				[this, targetX, targetZ, s, hm, targetLOD]()
				{
					MeshData data = Terrain::GenerateData(
						targetX, targetZ, s, hm, targetLOD
					);

					std::lock_guard<std::mutex> lock(this->queueMutex);
					this->completedChunksQueue.push(std::move(data));
				}
			));
		}
	}
}

void World::Draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec3& cameraPos, glm::vec3& sunDirection)
{
	terrainShader.Use();

	terrainShader.SetMat4("view", view);
	terrainShader.SetMat4("projection", projection);

	//terrainShader.SetVec3("viewPos", cameraPos);
	terrainShader.SetVec3("sunDir", sunDirection);
	terrainShader.SetFloat("waterLevel", waterLevel);

	terrainShader.SetFloat("textureScale", 0.025f);

	plainsTexture.Bind(0);
	desertTexture.Bind(1);
	forestTexture.Bind(2);
	hillsTexture.Bind(3);
	mountainsTexture.Bind(4);
	snowTexture.Bind(5);

	for (auto& pair : activeChunks)
	{
		int chunkX = pair.first.first;
		int chunkZ = pair.first.second;

		glm::mat4 chunkModel = glm::translate(model, glm::vec3(chunkX * chunkSize, terrainLevel, chunkZ * chunkSize));
		pair.second->Draw(chunkModel, terrainShader);
	}

	water.BeginDraw(view, projection, cameraPos);

	Shader& waterShader = water.GetShader();

	for (auto& [chunkPos, terrain] : activeChunks)
	{
		const int chunkX = chunkPos.first;
		const int chunkZ = chunkPos.second;

		glm::mat4 waterModel = glm::translate(
			model,
			glm::vec3(
				chunkX * chunkSize,
				waterLevel,
				chunkZ * chunkSize
			)
		);

		terrain->DrawWater(waterModel, waterShader);
	}
}

int World::CalculateLOD(int chunkX, int chunkZ, int playerX, int playerZ)
{
	int distX = std::abs(chunkX - playerX);
	int distZ = std::abs(chunkZ - playerZ);
	int maxDist = std::max(distX, distZ);

	if (maxDist <= 1) return 1;
	if (maxDist == 2) return 2;
	if (maxDist == 3) return 4;
	if (maxDist == 4) return 8;
	return 16;
}
