#include "Graphics/Model.hpp"

#include "cgltf.h"
#include <iostream>
#include <cassert>
#include <cstring>
#include <memory>
#include <utility>

std::shared_ptr<Texture> LoadTexture(cgltf_image* image)
{
	if (!image)
		return nullptr;

	if (!image->buffer_view)
	{
		std::cerr << "[ERROR] Texture image has no buffer view\n";
		return nullptr;
	}

	cgltf_buffer_view* view = image->buffer_view;

	const unsigned char* data =
		static_cast<const unsigned char*>(view->buffer->data)
		+ view->offset;

	std::size_t size = view->size;

	return std::make_shared<Texture>(data, size);
}

Model::Model(Model&& other) noexcept
	: meshes(std::move(other.meshes))
{
}

Model& Model::operator=(Model&& other) noexcept
{
	if (this != &other)
	{
		meshes = std::move(other.meshes);
	}

	return *this;
}

Model::Model(const char* file)
{
	std::printf("file path: %s\n", file);
	cgltf_options options;
	std::memset(&options, 0, sizeof(cgltf_options));
	cgltf_data* data = NULL;
	cgltf_result result = cgltf_parse_file(&options, file, &data);

	if (result == cgltf_result_success)
		result = cgltf_load_buffers(&options, data, file);

	if (result == cgltf_result_success)
		result = cgltf_validate(data);

	std::printf("Result: %d\n", result);

	if (result == cgltf_result_success)
	{
		std::printf("Type: %u\n", data->file_type);
		std::printf("Meshes: %u\n", (unsigned)data->meshes_count);
	}

	for (cgltf_size i = 0; i < data->meshes_count; i++)
	{
		cgltf_mesh& mesh = data->meshes[i];

		std::cout << "Mesh: " << (mesh.name ? mesh.name : "Unnamed") << std::endl;

		for (cgltf_size j = 0; j < mesh.primitives_count; j++)
		{
			cgltf_primitive& primitive = mesh.primitives[j];

			std::cout << "\nPrimitive: " << j << '\n';

			if (primitive.material)
			{
				std::cout << "  Material: "
					<< (primitive.material->name
						? primitive.material->name
						: "Unnamed")
					<< '\n';

				auto& baseColor =
					primitive.material->pbr_metallic_roughness.base_color_texture;

				if (baseColor.texture)
				{
					std::cout << "  Texture: "
						<< (baseColor.texture->name
							? baseColor.texture->name
							: "Unnamed")
						<< '\n';

					if (baseColor.texture->image)
					{
						std::cout << "  Image: "
							<< (baseColor.texture->image->name
								? baseColor.texture->image->name
								: "Unnamed")
							<< '\n';
					}
				}
				else
				{
					std::cout << "  No base color texture\n";
				}
			}
			else
			{
				std::cout << "  No material\n";
			}

			if (primitive.type != cgltf_primitive_type_triangles) {
				std::cout << "Primitive is not a triangle!";
				continue;
			}

			Material material;

			if (primitive.material)
			{
				cgltf_material* gltfMaterial = primitive.material;

				auto& pbr = gltfMaterial->pbr_metallic_roughness;

				// Base color
				material.baseColor = glm::vec4(
					pbr.base_color_factor[0],
					pbr.base_color_factor[1],
					pbr.base_color_factor[2],
					pbr.base_color_factor[3]
				);

				// Base color texture
				if (pbr.base_color_texture.texture)
				{
					cgltf_image* image =
						pbr.base_color_texture.texture->image;

					if (image)
					{
						material.baseColorTexture = LoadTexture(image);
					}
				}
			}

			cgltf_accessor* positionAccessor = nullptr;
			cgltf_accessor* normalAccessor = nullptr;
			cgltf_accessor* texCoordAccessor = nullptr;

			for (cgltf_size k = 0; k < primitive.attributes_count; k++)
			{
				cgltf_attribute& attribute = primitive.attributes[k];

				if (attribute.type == cgltf_attribute_type_position)
					positionAccessor = attribute.data;
				else if (attribute.type == cgltf_attribute_type_normal)
					normalAccessor = attribute.data;
				else if (attribute.type == cgltf_attribute_type_texcoord)
					texCoordAccessor = attribute.data;
			}

			std::vector<Vertex> vertices;

			if (positionAccessor == nullptr) {
				// handle error: log, return, or throw depending on function contract
				return; // or throw std::runtime_error("positionAccessor is null");
			}
			vertices.reserve(positionAccessor->count);

			for (cgltf_size v = 0; v < positionAccessor->count; v++)
			{
				float position[3] = {};

				cgltf_accessor_read_float(
					positionAccessor,
					v,
					position,
					3
				);

				Vertex vertex{};

				vertex.position = glm::vec3(
					position[0],
					position[1],
					position[2]
				);

				if (normalAccessor)
				{
					float normal[3] = {};

					cgltf_accessor_read_float(
						normalAccessor,
						v,
						normal,
						3
					);

					vertex.normal = glm::vec3(
						normal[0],
						normal[1],
						normal[2]
					);
				}

				if (texCoordAccessor)
				{
					float uv[2] = {};

					cgltf_accessor_read_float(
						texCoordAccessor,
						v,
						uv,
						2
					);

					vertex.texCoord = glm::vec2(
						uv[0],
						uv[1]
					);
				}



				vertices.push_back(vertex);
			}

			std::vector<unsigned int> indices;

			if (primitive.indices)
			{
				indices.reserve(primitive.indices->count);

				for (cgltf_size p = 0;
					p < primitive.indices->count;
					p++)
				{
					indices.push_back(
						static_cast<unsigned int>(
							cgltf_accessor_read_index(
								primitive.indices,
								p
							)
							)
					);
				}
			}

			meshes.emplace_back(vertices, indices, material);
		}
	}

	cgltf_free(data);
}

Model::~Model()
{
}

void Model::Draw(Shader& shader) const
{
	//int count = 0;

	for (const Mesh& mesh : meshes)
	{
		//count++;
		//glm::vec4 color;

		//if (count == 1) {
		//	color = glm::vec4(1, 0, 0, 1);
		//}
		//else {
		//	color = glm::vec4(0, 1, 0, 1);
		//	continue;
		//}

		//std::cout << count << "    " << color.r << std::endl;

		//shader.SetVec4("colorUni", color);

		mesh.Draw(shader);
	}
}

/*
int Model::LoadModel(const char* file)
{
	return 0;
}
*/