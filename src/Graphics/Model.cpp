#include "Graphics/Model.hpp"

#include "cgltf.h"

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

	cgltf_free(data);
}

Model::~Model()
{
}

/*
int Model::LoadModel(const char* file)
{
	return 0;
}
*/