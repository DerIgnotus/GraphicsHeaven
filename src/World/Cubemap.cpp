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


Cubemap::~Cubemap()
{
}

void Cubemap::Render(glm::mat4 view, glm::mat4 projection) {
	glDepthFunc(GL_LEQUAL);

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

	glDepthFunc(GL_LESS);
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

