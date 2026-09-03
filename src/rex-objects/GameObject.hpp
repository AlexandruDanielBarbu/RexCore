#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

#include <optional>

struct Mesh
{
	uint32_t vertexOffset;
	uint32_t vertexCount;
	uint32_t indexOffset;
	uint32_t indexCount;
};

struct GameObject
{
	glm::vec3 position{};
	glm::vec3 rotation{};
	glm::vec3 scale{1,1,1};

	std::vector<vk::raii::Buffer>       uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void *>                 uniformBuffersMapped;

	std::vector<vk::raii::DescriptorSet> descriptorSets;

	glm::mat4 getModelMatrix() const
	{
		glm::mat4 model = glm::mat4(1.0f);

		model = glm::translate(model, position);

		model = glm::rotate(model, rotation.x, glm::vec3(1.0f, 0.0f, 0.0f));
		model = glm::rotate(model, rotation.y, glm::vec3(0.0f, 1.0f, 0.0f));
		model = glm::rotate(model, rotation.z, glm::vec3(0.0f, 0.0f, 1.0f));

		model = glm::scale(model, scale);

		return model;
	}

	// Optional GameObject data
	std::optional<Mesh> mesh;
};