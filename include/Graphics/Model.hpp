#pragma once

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>
#include <memory>
#include <cgltf.h>

class Model {
public:
	Model(const char* file);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;

	void Draw(Shader& shader, unsigned int skyboxTexture) const;

private:
	std::shared_ptr<Texture> LoadTexture(
		cgltf_image* image,
		TextureType type
	);

	std::string modelDirectory;

	std::vector<Mesh> meshes;
};