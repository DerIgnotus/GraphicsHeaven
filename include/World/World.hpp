#pragma once
#include <map>
#include <set>
#include <queue>
#include <mutex>
#include <future>
#include <utility>
#include <memory>

#include "World/Terrain.hpp"
#include "Graphics/Shader.hpp"
#include "Graphics/Texture.hpp"
#include "World/Water.hpp"

class World {
public:
	//World();
	~World();

	World(int seed, float heightMultiplier, int lod = 1);

	void Update(const glm::vec3& cameraPos);
	void Draw(glm::mat4& model, glm::mat4& view, glm::mat4& projection, const glm::vec3& cameraPos, glm::vec3& sunDirection);

private:
	float waterLevel = 0.0f;
	Water water{ waterLevel };
	float terrainLevel = 30.0f;

	int CalculateLOD(int chunkX, int chunkZ, int playerX, int playerZ);

	Shader terrainShader;

	Texture sandTexture;
	Texture grassTexture;
	Texture rockTexture;
	Texture snowTexture;

	std::map<std::pair<int, int>, int> activeChunkLODs;

	int seed = 1;
	float heightMultiplier = 1.0f;
	int lod = 1;

	const int chunkSize = 240;      // (mapChunkSize - 1)
	const int renderDistance = 8;   // ((renderDistance * 2) + 1) * ((renderDistance * 2) + 1)

	std::map<std::pair<int, int>, std::unique_ptr<Terrain>> activeChunks;
	std::set<std::pair<int, int>> loadingChunks;

	std::queue<MeshData> completedChunksQueue;
	std::mutex queueMutex;

	std::vector<std::future<void>> backgroundTasks;

	int lastPlayerChunkX = -999;
	int lastPlayerChunkZ = -999;
};