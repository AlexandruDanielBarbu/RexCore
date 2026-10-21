#pragma once

#include <glm/glm.hpp>

struct Texture;

struct Material
{
	std::string name;
	Texture albedo;
	Texture normal_map;

	glm::vec4 ambient    = {};
	glm::vec4 diffuse    = {};
	glm::vec4 specular   = {};
	float     shininess  = 0.5f;
};

struct SubMeshMaterial
{
	uint32_t firstIndex;
	uint32_t indexCount;
	int      materialId = -1;         // -1 = none
};