#pragma once

#include <glad/glad.h>

#include <cstddef>

class Texture
{
public:
	Texture(const unsigned char* data, std::size_t size);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetID() const { return ID; }

private:
	unsigned int ID = 0;
};