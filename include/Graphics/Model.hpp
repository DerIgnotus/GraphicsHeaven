#pragma once

#include "Shader.hpp"
#include "Mesh.hpp"

#include <cstring>
#include <cstdio>
#include <vector>
#include <string>

class Model {
public:
	Model(const char* file);
	~Model();

	//int LoadModel(const char* file);

	void Draw(Shader& shader) const;

private:
	std::vector<Mesh> meshes;
};