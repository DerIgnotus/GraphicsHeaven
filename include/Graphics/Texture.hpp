#pragma once

#include <glad/glad.h>

#include <cstddef>

enum class TextureType
{
	Color,
	Data
};

class Texture
{
public:
	Texture(const unsigned char* data, std::size_t size);
	Texture(const unsigned char* data, size_t size, TextureType type);
	Texture(const char* path, TextureType type);
	~Texture();

	void Bind(unsigned int slot = 0) const;
	void Unbind() const;

	unsigned int GetID() const { return ID; }

private:
	unsigned int ID = 0;
};