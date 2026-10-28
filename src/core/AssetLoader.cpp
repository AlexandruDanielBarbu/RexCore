#include "AssetLoader.hpp"
#include <iostream>
#include <fstream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define TINYOBJLOADER_IMPLEMENTATION
#include <tiny_obj_loader.h>

#include "spdlog/spdlog.h"


namespace rex::AssetLoader
{
	namespace
	{
		#ifdef NDEBUG
		#	define ASSET(p) "assets/" p
		#else
		#	define ASSET(p) REXCORE_ASSETS "/assets/" p
		#endif        // NDEBUG

		void insertIntoSubMeshVector(ModelData& model, const int matId)
		{
			std::vector<SubMeshMaterial> &subMeshes = model.submeshes;

			if (!subMeshes.empty())
			{
				SubMeshMaterial &lastSubmesh = subMeshes[subMeshes.size() - 1];

				if (lastSubmesh.materialId == matId)
				{
					lastSubmesh.indexCount += 3;
					return;
				}
			}

			SubMeshMaterial smm{
				.firstIndex = static_cast<uint32_t>(model.indices.size()),
				.indexCount = 3,
				.materialId = matId};

			subMeshes.push_back(smm);
		}
	}

	std::vector<char> readFile(const std::string &fileName)
	{
		std::ifstream file(fileName, std::ios::ate | std::ios::binary);

		if (!file.is_open())
		{
			throw std::runtime_error("failed to open file!");
		}

		std::vector<char> buffer(file.tellg());

		file.seekg(0, std::ios::beg);
		file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

		file.close();

		return buffer;
	}

	ImageData loadImage(const std::string &path)
	{
	    int            texWidth, texHeight, texChannels;
	    stbi_uc       *pixels    = stbi_load(path.c_str(), &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);

	    if (!pixels)
		    throw std::runtime_error("failed to load texture image!");
		
	    ImageData img{
	        .width  = static_cast<uint32_t>(texWidth),
	        .height = static_cast<uint32_t>(texHeight),
	    };
	    img.pixels.assign(pixels, pixels + static_cast<size_t>(texWidth) * texHeight * 4);
	    
	    stbi_image_free(pixels);

	    return img;
    }

	ModelData loadModel(const std::string &path)
	{
		ModelData mdl{};

		tinyobj::attrib_t                attrib;
		std::vector<tinyobj::shape_t>    shapes;
		std::vector<tinyobj::material_t> materials;
		std::string                      warn, err;

		// tinyobj error handling
		{
			if (!tinyobj::LoadObj(&attrib, &shapes, &materials, &warn, &err, path.c_str()))
				throw std::runtime_error(warn + err);

			if (!warn.empty())
				spdlog::warn(warn);
			
			if (!err.empty())
				spdlog::error(err);
		}
		
		// adding model textures
		std::string assetRoot = ASSET("");
		for (size_t i = 0; i < materials.size(); i++)
		{
			mdl.materialTextures.push_back(assetRoot + materials[i].diffuse_texname);
		}
		
		// adding model
		std::unordered_map<Vertex, uint32_t> uniqueVertices{};
		for (const auto &shape : shapes)
		{
			for (size_t f = 0; f < shape.mesh.material_ids.size(); f++)
			{
				// inserting submeshes
				int matId = shape.mesh.material_ids[f];
				insertIntoSubMeshVector(mdl, matId);
				
				// filling up vertex and index buffer
				for (int k = 0; k < 3; ++k)
				{
					const auto &index = shape.mesh.indices[3 * f + k];

					Vertex vertex{};
					vertex.pos = {
					attrib.vertices[3 * index.vertex_index + 0],
					attrib.vertices[3 * index.vertex_index + 1],
					attrib.vertices[3 * index.vertex_index + 2]};

					assert(index.texcoord_index >= 0);
					vertex.texCoord = {
					attrib.texcoords[2 * index.texcoord_index + 0],
					1.0f - attrib.texcoords[2 * index.texcoord_index + 1]};

					vertex.color = {1.0f, 1.0f, 1.0f};

					auto [it, inserted] = uniqueVertices.insert({vertex, static_cast<uint32_t>(mdl.vertices.size())});
					if (inserted)
					{
						mdl.vertices.push_back(vertex);
					}

					mdl.indices.push_back(it->second);
				}
			}
		}

		spdlog::info("Model has: {0} vertices.", mdl.vertices.size());

		return mdl;
	}

}