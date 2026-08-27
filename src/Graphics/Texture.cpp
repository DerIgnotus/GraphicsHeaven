#include "Graphics/Texture.hpp"

#include <iostream>

#include "stb_image.h"

Texture::Texture(const unsigned char* data, std::size_t size)
{
	int width;
	int height;
	int channels;

	unsigned char* pixels = stbi_load_from_memory(
		data,
		static_cast<int>(size),
		&width,
		&height,
		&channels,
		0
	);

	if (!pixels)
	{
		std::cerr << "[ERROR] Failed to decode texture: "
			<< stbi_failure_reason() << '\n';
		return;
	}

	GLenum format;

	switch (channels)
	{
	case 1:
		format = GL_RED;
		break;

	case 3:
		format = GL_RGB;
		break;

	case 4:
		format = GL_RGBA;
		break;

	default:
		std::cerr << "[ERROR] Unsupported texture format: "
			<< channels << " channels\n";

		stbi_image_free(pixels);
		return;
	}

	glGenTextures(1, &ID);
	glBindTexture(GL_TEXTURE_2D, ID);

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		format,
		width,
		height,
		0,
		format,
		GL_UNSIGNED_BYTE,
		pixels
	);

	glGenerateMipmap(GL_TEXTURE_2D);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_S,
		GL_REPEAT
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_WRAP_T,
		GL_REPEAT
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_2D,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	glBindTexture(GL_TEXTURE_2D, 0);

	stbi_image_free(pixels);

	std::cout << "[ OK ] Texture created: "
		<< width << "x" << height << '\n';
}

Texture::~Texture()
{
	if (ID != 0)
		glDeleteTextures(1, &ID);
}

void Texture::Bind(unsigned int slot) const
{
	glActiveTexture(GL_TEXTURE0 + slot);
	glBindTexture(GL_TEXTURE_2D, ID);
}

void Texture::Unbind() const
{
	glBindTexture(GL_TEXTURE_2D, 0);
}