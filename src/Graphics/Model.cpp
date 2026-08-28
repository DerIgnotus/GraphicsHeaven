#include "Graphics/Model.hpp"

#include <iostream>
#include <cassert>
#include <cstring>
#include <utility>

std::shared_ptr<Texture> Model::LoadTexture(
	cgltf_image* image,
	TextureType type
)
{
	if (!image)
		return nullptr;

	// External texture
	if (image->uri)
	{
		std::string path = modelDirectory + image->uri;

		return std::make_shared<Texture>(
			path.c_str(),
			type
		);
	}

	// Embedded texture in GLB
	if (image->buffer_view)
	{
		cgltf_buffer_view* view = image->buffer_view;

		const unsigned char* data =
			static_cast<const unsigned char*>(
				view->buffer->data
				) + view->offset;

		return std::make_shared<Texture>(
			data,
			view->size,
			type
		);
	}

	std::cerr << "Texture has no data: "
		<< (image->name ? image->name : "Unnamed")
		<< '\n';

	return nullptr;
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

	std::string filePath(file);

	size_t slash = filePath.find_last_of("/\\");

	if (slash != std::string::npos)
		modelDirectory = filePath.substr(0, slash + 1);
	else
		modelDirectory = "";

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

			if (primitive.type != cgltf_primitive_type_triangles) {
				std::cout << "Primitive is not a triangle!";
				continue;
			}

			Material material;

			if (primitive.material)
			{
				cgltf_material* gltfMaterial = primitive.material;

				auto& pbr = gltfMaterial->pbr_metallic_roughness;

				std::cout << "\n============================\n";
				std::cout << "Material: "
					<< (gltfMaterial->name ? gltfMaterial->name : "Unnamed")
					<< '\n';

				std::cout << "Base color texture: "
					<< (pbr.base_color_texture.texture ? "YES" : "NO")
					<< '\n';

				std::cout << "Metal/Rough texture: "
					<< (pbr.metallic_roughness_texture.texture ? "YES" : "NO")
					<< '\n';

				std::cout << "Normal texture: "
					<< (gltfMaterial->normal_texture.texture ? "YES" : "NO")
					<< '\n';

				std::cout << "AO texture: "
					<< (gltfMaterial->occlusion_texture.texture ? "YES" : "NO")
					<< '\n';

				std::cout << "Emissive texture: "
					<< (gltfMaterial->emissive_texture.texture ? "YES" : "NO")
					<< '\n';

				std::cout << "Metallic factor: "
					<< pbr.metallic_factor
					<< '\n';

				std::cout << "Roughness factor: "
					<< pbr.roughness_factor
					<< '\n';

				std::cout << "============================\n";

				// Base color
				material.baseColor = glm::vec4(
					pbr.base_color_factor[0],
					pbr.base_color_factor[1],
					pbr.base_color_factor[2],
					pbr.base_color_factor[3]
				);

				// Metallic / roughness factors
				material.metallic = pbr.metallic_factor;
				material.roughness = pbr.roughness_factor;

				// Base color texture
				if (pbr.base_color_texture.texture)
				{
					cgltf_image* image = pbr.base_color_texture.texture->image;

					if (image)
						material.baseColorTexture = LoadTexture(image, TextureType::Color);
				}

				// Metallic + roughness texture
				if (pbr.metallic_roughness_texture.texture)
				{
					cgltf_image* image = pbr.metallic_roughness_texture.texture->image;

					if (image)
						material.metallicRoughnessTexture = LoadTexture(image, TextureType::Data);
				}

				// Normal texture
				if (gltfMaterial->normal_texture.texture)
				{
					cgltf_image* image = gltfMaterial->normal_texture.texture->image;

					if (image)
						material.normalTexture = LoadTexture(image, TextureType::Data);
				}

				// Occlusion texture
				if (gltfMaterial->occlusion_texture.texture)
				{
					cgltf_image* image = gltfMaterial->occlusion_texture.texture->image;

					if (image)
						material.occlusionTexture = LoadTexture(image, TextureType::Data);
				}

				// Emissive texture
				if (gltfMaterial->emissive_texture.texture)
				{
					cgltf_image* image = gltfMaterial->emissive_texture.texture->image;

					if (image)
						material.emissiveTexture = LoadTexture(image, TextureType::Color);
				}

				// Emissive factor
				material.emissive = glm::vec3(
					gltfMaterial->emissive_factor[0],
					gltfMaterial->emissive_factor[1],
					gltfMaterial->emissive_factor[2]
				);
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

	std::cout << "Model Loaded Successfully: " << (std::string)file << std::endl;

	cgltf_free(data);
}

Model::~Model()
{
}

void Model::Draw(Shader& shader, unsigned int skyboxTexture) const
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

		mesh.Draw(shader, skyboxTexture);
	}
}

/*
int Model::LoadModel(const char* file)
{
	return 0;
}
*/