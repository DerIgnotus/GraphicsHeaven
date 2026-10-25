#include "World/World.hpp"
#include <cmath>
#include <glm/ext/matrix_transform.hpp>

World::World() {}
World::~World() {}

World::World(int seed, float heightMultiplier, int lod) :
	seed(seed), heightMultiplier(heightMultiplier),
	lod(lod), terrainShader("shaders/terrain.vert", "shaders/terrain.frag")
{
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
		if (requiredChunks.find(it->first) == requiredChunks.end()) {
			it = activeChunks.erase(it);
		}
		else {
			++it;
		}
	}

	for (const auto& key : requiredChunks)
	{
		if (activeChunks.find(key) == activeChunks.end() && loadingChunks.find(key) == loadingChunks.end())
		{
			loadingChunks.insert(key);

			int targetX = key.first;
			int targetZ = key.second;
			int s = seed;
			float hm = heightMultiplier;
			int l = lod;

			backgroundTasks.push_back(std::async(std::launch::async, [this, targetX, targetZ, s, hm, l]() {
				MeshData data = Terrain::GenerateData(targetX, targetZ, s, hm, l);

				std::lock_guard<std::mutex> lock(this->queueMutex);
				this->completedChunksQueue.push(std::move(data));
				}));
		}
	}
}

void World::Draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection)
{
	for (auto& pair : activeChunks)
	{
		int chunkX = pair.first.first;
		int chunkZ = pair.first.second;

		glm::mat4 chunkModel = glm::translate(model, glm::vec3(chunkX * chunkSize, 0.0f, chunkZ * chunkSize));
		pair.second->Draw(chunkModel, view, projection, terrainShader);
	}
}