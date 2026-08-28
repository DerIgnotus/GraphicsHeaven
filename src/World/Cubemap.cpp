#include "World/Cubemap.hpp"
#include <glm/ext/matrix_transform.hpp>
#include <glm/ext/matrix_clip_space.hpp>

Cubemap::Cubemap()
{
	skyboxVAO = 0;
	skyboxVBO = 0;
}

Cubemap::Cubemap(const char* path, const char* format)
{
	std::string pathString(path);
	std::string formatString(format);

	std::vector<std::string> faces
	{
		pathString + "right." + formatString,
		pathString + "left." + formatString,
		pathString + "top." + formatString,
		pathString + "bottom." + formatString,
		pathString + "front." + formatString,
		pathString + "back." + formatString
	};

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	int width, height, nrChannels;

	for (unsigned int i = 0; i < faces.size(); i++)
	{
		unsigned char* data = stbi_load(
			faces[i].c_str(),
			&width,
			&height,
			&nrChannels,
			0
		);

		if (data)
		{
			GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;

			glTexImage2D(
				GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
				0,
				format,
				width,
				height,
				0,
				format,
				GL_UNSIGNED_BYTE,
				data
			);

			stbi_image_free(data);
		}
		else
		{
			std::cout
				<< "Cubemap tex failed to load at path: "
				<< faces[i]
				<< std::endl;
		}
	}

	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_CUBE_MAP, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE
	);

	setupSkybox();
}

Cubemap Cubemap::FromHDRI(const char* path)
{
	Cubemap result;

	result.setupSkybox();
	result.loadHDRI(path);

	return result;
}

Cubemap Cubemap::FromSingleFile(const char* path)
{
	Cubemap result;

	result.loadCrossCubemap(path);
	result.setupSkybox();

	return result;
}

Cubemap::~Cubemap()
{
}

void Cubemap::Render(glm::mat4 view, glm::mat4 projection) {
	glDepthFunc(GL_LEQUAL); // change depth function so depth test passes when values are equal to depth buffer's content

	skyboxShader.Use();
	glm::mat4 skyboxView = glm::mat4(glm::mat3(view));
	skyboxShader.SetMat4("view", skyboxView);
	skyboxShader.SetMat4("projection", projection);
	// skybox cube
	glBindVertexArray(skyboxVAO);
	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);
	glDrawArrays(GL_TRIANGLES, 0, 36);
	glBindVertexArray(0);

	glDepthFunc(GL_LESS); // set depth function back to default
}

void Cubemap::setupSkybox()
{
	skyboxShader = Shader(
		"shaders/skybox.vert",
		"shaders/skybox.frag"
	);

	skyboxShader.Use();
	skyboxShader.SetInt("skybox", 0);

	glGenVertexArrays(1, &skyboxVAO);
	glGenBuffers(1, &skyboxVBO);

	glBindVertexArray(skyboxVAO);

	glBindBuffer(GL_ARRAY_BUFFER, skyboxVBO);
	glBufferData(
		GL_ARRAY_BUFFER,
		sizeof(skyboxVertices),
		skyboxVertices,
		GL_STATIC_DRAW
	);

	glEnableVertexAttribArray(0);
	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		3 * sizeof(float),
		(void*)0
	);

	glBindVertexArray(0);

	std::cout << "Skybox Created Successfully" << std::endl;
}

void Cubemap::loadHDRI(const char* path)
{
	GLint oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);

	stbi_set_flip_vertically_on_load(true);

	int width, height, nrComponents;

	float* data = stbi_loadf(
		path,
		&width,
		&height,
		&nrComponents,
		0
	);

	if (!data)
	{
		std::cerr << "Failed to load HDR: " << path << '\n';
		stbi_set_flip_vertically_on_load(false);
		return;
	}

	unsigned int hdrTexture;

	glGenTextures(1, &hdrTexture);
	glBindTexture(GL_TEXTURE_2D, hdrTexture);

	GLenum format = nrComponents == 4 ? GL_RGBA : GL_RGB;
	GLenum internalFormat = nrComponents == 4 ? GL_RGBA16F : GL_RGB16F;

	glTexImage2D(
		GL_TEXTURE_2D,
		0,
		internalFormat,
		width,
		height,
		0,
		format,
		GL_FLOAT,
		data
	);

	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

	stbi_image_free(data);
	stbi_set_flip_vertically_on_load(false);

	convertEquirectangularToCubemap(hdrTexture, 1024);

	glDeleteTextures(1, &hdrTexture);

	glViewport(
		oldViewport[0],
		oldViewport[1],
		oldViewport[2],
		oldViewport[3]
	);

	std::cout << "HDRI converted to cubemap successfully!\n";
}

void Cubemap::loadCrossCubemap(const char* path)
{
	int width, height, nrChannels;

	unsigned char* data = stbi_load(
		path,
		&width,
		&height,
		&nrChannels,
		0
	);

	if (!data)
	{
		std::cerr << "Failed to load cubemap: " << path << '\n';
		return;
	}

	int faceWidth = width / 4;
	int faceHeight = height / 3;

	if (faceWidth != faceHeight)
	{
		std::cerr << "Invalid cubemap cross dimensions\n";
		stbi_image_free(data);
		return;
	}

	GLenum format = nrChannels == 4 ? GL_RGBA : GL_RGB;

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	struct Face
	{
		int x;
		int y;
	};

	Face faces[6] =
	{
		{ 2, 1 }, // +X RIGHT
		{ 0, 1 }, // -X LEFT
		{ 1, 0 }, // +Y TOP
		{ 1, 2 }, // -Y BOTTOM
		{ 1, 1 }, // +Z FRONT
		{ 3, 1 }  // -Z BACK
	};

	for (int face = 0; face < 6; face++)
	{
		std::vector<unsigned char> faceData(
			faceWidth * faceHeight * nrChannels
		);

		for (int y = 0; y < faceHeight; y++)
		{
			memcpy(
				&faceData[y * faceWidth * nrChannels],
				&data[
					((faces[face].y * faceHeight + y) * width
						+ faces[face].x * faceWidth)
						* nrChannels
				],
				faceWidth * nrChannels
			);
		}

		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + face,
			0,
			format,
			faceWidth,
			faceHeight,
			0,
			format,
			GL_UNSIGNED_BYTE,
			faceData.data()
		);
	}

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR_MIPMAP_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE
	);

	glGenerateMipmap(GL_TEXTURE_CUBE_MAP);

	stbi_image_free(data);

	std::cout << "Cross cubemap loaded: "
		<< width << "x" << height << '\n';
}

void Cubemap::convertEquirectangularToCubemap(
	unsigned int equirectangularTexture,
	unsigned int resolution
)
{
	GLint oldViewport[4];
	glGetIntegerv(GL_VIEWPORT, oldViewport);

	glGenTextures(1, &cubemapTexture);
	glBindTexture(GL_TEXTURE_CUBE_MAP, cubemapTexture);

	for (unsigned int i = 0; i < 6; i++)
	{
		glTexImage2D(
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			0,
			GL_RGB16F,
			resolution,
			resolution,
			0,
			GL_RGB,
			GL_FLOAT,
			nullptr
		);
	}

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_S,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_T,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_WRAP_R,
		GL_CLAMP_TO_EDGE
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MIN_FILTER,
		GL_LINEAR
	);

	glTexParameteri(
		GL_TEXTURE_CUBE_MAP,
		GL_TEXTURE_MAG_FILTER,
		GL_LINEAR
	);

	unsigned int captureFBO;
	unsigned int captureRBO;

	glGenFramebuffers(1, &captureFBO);
	glGenRenderbuffers(1, &captureRBO);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	glBindRenderbuffer(GL_RENDERBUFFER, captureRBO);

	glRenderbufferStorage(
		GL_RENDERBUFFER,
		GL_DEPTH_COMPONENT24,
		resolution,
		resolution
	);

	glFramebufferRenderbuffer(
		GL_FRAMEBUFFER,
		GL_DEPTH_ATTACHMENT,
		GL_RENDERBUFFER,
		captureRBO
	);

	Shader equirectangularShader(
		"shaders/equirectangular.vert",
		"shaders/equirectangular.frag"
	);

	equirectangularShader.Use();

	equirectangularShader.SetInt(
		"equirectangularMap",
		0
	);

	glm::mat4 captureProjection = glm::perspective(
		glm::radians(90.0f),
		1.0f,
		0.1f,
		10.0f
	);

	glm::mat4 captureViews[] =
	{
		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),

		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(-1.0f, 0.0f, 0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),

		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(0.0f, 1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f)
		),

		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(0.0f, -1.0f, 0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f)
		),

		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(0.0f, 0.0f, 1.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		),

		glm::lookAt(
			glm::vec3(0.0f),
			glm::vec3(0.0f, 0.0f, -1.0f),
			glm::vec3(0.0f, -1.0f, 0.0f)
		)
	};

	equirectangularShader.Use();

	equirectangularShader.SetMat4(
		"projection",
		captureProjection
	);

	glActiveTexture(GL_TEXTURE0);
	glBindTexture(GL_TEXTURE_2D, equirectangularTexture);

	glViewport(0, 0, resolution, resolution);

	glBindFramebuffer(GL_FRAMEBUFFER, captureFBO);

	for (unsigned int i = 0; i < 6; i++)
	{
		equirectangularShader.SetMat4(
			"view",
			captureViews[i]
		);

		glFramebufferTexture2D(
			GL_FRAMEBUFFER,
			GL_COLOR_ATTACHMENT0,
			GL_TEXTURE_CUBE_MAP_POSITIVE_X + i,
			cubemapTexture,
			0
		);

		glClear(
			GL_COLOR_BUFFER_BIT |
			GL_DEPTH_BUFFER_BIT
		);

		glBindVertexArray(skyboxVAO);
		glDrawArrays(GL_TRIANGLES, 0, 36);
		glBindVertexArray(0);
	}

	glBindFramebuffer(GL_FRAMEBUFFER, 0);

	glViewport(
		oldViewport[0],
		oldViewport[1],
		oldViewport[2],
		oldViewport[3]
	);

	glDeleteFramebuffers(1, &captureFBO);
	glDeleteRenderbuffers(1, &captureRBO);
}