#pragma once

#include "Shader.hpp"
#include "Mesh.hpp"
#include "Texture.hpp"

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

class Model {
public:
	Model(const char* file);
	~Model();

	Model(const Model&) = delete;
	Model& operator=(const Model&) = delete;

	Model(Model&& other) noexcept;
	Model& operator=(Model&& other) noexcept;

	void Draw(Shader& shader) const;

private:
	std::vector<Mesh> meshes;
};