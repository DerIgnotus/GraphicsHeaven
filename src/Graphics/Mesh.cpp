#include "Graphics/Mesh.hpp"

#include <utility>

Mesh::Mesh(
	const std::vector<Vertex>& vertices,
	const std::vector<unsigned int>& indices,
	const Material& material
) :
	vertices(vertices),
	indices(indices),
	material(material)
{
	setupMesh();
}

Mesh::Mesh(Mesh&& other) noexcept
	: vertices(std::move(other.vertices)),
	indices(std::move(other.indices)),
	material(std::move(other.material)),
	VAO(other.VAO),
	VBO(other.VBO),
	EBO(other.EBO)
{
	other.VAO = 0;
	other.VBO = 0;
	other.EBO = 0;
}

Mesh& Mesh::operator=(Mesh&& other) noexcept
{
	if (this != &other)
	{
		glDeleteVertexArrays(1, &VAO);
		glDeleteBuffers(1, &VBO);
		glDeleteBuffers(1, &EBO);

		vertices = std::move(other.vertices);
		indices = std::move(other.indices);
		material = std::move(other.material);

		VAO = other.VAO;
		VBO = other.VBO;
		EBO = other.EBO;

		other.VAO = 0;
		other.VBO = 0;
		other.EBO = 0;
	}

	return *this;
}

Mesh::~Mesh()
{
	glDeleteVertexArrays(1, &VAO);
	glDeleteBuffers(1, &VBO);
	glDeleteBuffers(1, &EBO);
}


void Mesh::Draw(Shader& shader, unsigned int skyBox) const
{
	shader.SetVec4("baseColor", material.baseColor);
	shader.SetFloat("metallic", material.metallic);
	shader.SetFloat("roughness", material.roughness);
	shader.SetVec3("emissive", material.emissive);

	shader.SetBool("hasBaseColorTexture",
		material.baseColorTexture != nullptr);

	shader.SetBool("hasMetallicRoughnessTexture",
		material.metallicRoughnessTexture != nullptr);

	shader.SetBool("hasNormalTexture",
		material.normalTexture != nullptr);

	shader.SetBool("hasOcclusionTexture",
		material.occlusionTexture != nullptr);

	shader.SetBool("hasEmissiveTexture",
		material.emissiveTexture != nullptr);


	if (material.baseColorTexture)
		material.baseColorTexture->Bind(0);

	if (material.metallicRoughnessTexture)
		material.metallicRoughnessTexture->Bind(1);

	if (material.normalTexture)
		material.normalTexture->Bind(2);

	if (material.occlusionTexture)
		material.occlusionTexture->Bind(3);

	if (material.emissiveTexture)
		material.emissiveTexture->Bind(4);


	shader.SetInt("baseColorTexture", 0);
	shader.SetInt("metallicRoughnessTexture", 1);
	shader.SetInt("normalTexture", 2);
	shader.SetInt("occlusionTexture", 3);
	shader.SetInt("emissiveTexture", 4);


	glBindVertexArray(VAO);

	glDrawElements(
		GL_TRIANGLES,
		static_cast<GLsizei>(indices.size()),
		GL_UNSIGNED_INT,
		nullptr
	);

	glBindVertexArray(0);
}


/*
void Mesh::Draw(Shader& shader) const
{
	std::cout << "Drawing mesh: "
		<< vertices.size() << " vertices, "
		<< indices.size() << " indices\n";

	glBindVertexArray(VAO);

	glDrawElements(
		GL_TRIANGLES,
		static_cast<GLsizei>(indices.size()),
		GL_UNSIGNED_INT,
		nullptr
	);

	glBindVertexArray(0);
}
*/


void Mesh::setupMesh()
{
	std::cout << indices.size() << std::endl;
	std::cout << vertices.size() << std::endl;

	glGenVertexArrays(1, &VAO);
	glGenBuffers(1, &VBO);
	glGenBuffers(1, &EBO);

	glBindVertexArray(VAO);

	glBindBuffer(GL_ARRAY_BUFFER, VBO);
	glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);

	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, EBO);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);

	glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, position));
	glEnableVertexAttribArray(0);

	glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, normal));
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void*)offsetof(Vertex, texCoord));
	glEnableVertexAttribArray(2);

	glBindVertexArray(0);
}
