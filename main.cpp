#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#define VULKAN_HPP_HANDLE_ERROR_OUT_OF_DATE_AS_SUCCESS
#include <vulkan/vulkan_raii.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <chrono>
#include <iostream>  // report and propagate erros
#include <stdexcept> // report and propagate erros
#include <cstdlib>   // EXIT_FAILURE; EXIT_SUCCESS
#include <algorithm> // std::ranges
#include <vector>    // std::vector
#include <limits>    // std::numeric_limits
#include <algorithm> // std::clamp
#include <fstream>   // for shader binary loading
#include <filesystem>

constexpr uint32_t WIDTH          = 800;
constexpr uint32_t HEIGHT         = 800;
constexpr char     WINDOW_TITLE[] = "Rex Core";
constexpr int      MAX_FRAMES_IN_FLIGHT = 2;

const std::array<char const *, 1> validationLayers = {"VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif        // NDEBUG


// Validation layer debug printing callback
static VKAPI_ATTR vk::Bool32 VKAPI_CALL debugCallback(vk::DebugUtilsMessageSeverityFlagBitsEXT      severity,
                                                      vk::DebugUtilsMessageTypeFlagsEXT             type,
                                                      const vk::DebugUtilsMessengerCallbackDataEXT *pCallbackData,
                                                      void                                         *pUserData)
{
	std::cerr << "validation layer:\n\ttype " << to_string(type) << "\n\tmsg: " << pCallbackData->pMessage << std::endl;

	return vk::False;
}


static std::vector<char> readFile(const std::string &fileName)
{
	std::ifstream file(fileName, std::ios::ate | std::ios::binary);

	if (!file.is_open()) {
		throw std::runtime_error("failed to open file!");
	}

	std::vector<char> buffer(file.tellg());
	
	file.seekg(0, std::ios::beg);
	file.read(buffer.data(), static_cast<std::streamsize>(buffer.size()));

	file.close();

	return buffer;
}

struct UniformBufferObject
{
	alignas(16) glm::mat4 model;
	alignas(16) glm::mat4 view;
	alignas(16) glm::mat4 proj;
};

struct  Vertex
{
	glm::vec2 pos;
	glm::vec3 color;
	glm::vec2 texCoord;

	static vk::VertexInputBindingDescription getVertexBindingDescription()
	{
		return {
		    .binding   = 0,
		    .stride    = sizeof(Vertex),
		    .inputRate = vk::VertexInputRate::eVertex};
	}

	static std::array<vk::VertexInputAttributeDescription, 3> getVertexAttributeDescription()
	{
		return {{{.location = 0, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, pos)},
		         {.location = 1, .binding = 0, .format = vk::Format::eR32G32B32Sfloat, .offset = offsetof(Vertex, color)},
		         {.location = 2, .binding = 0, .format = vk::Format::eR32G32Sfloat, .offset = offsetof(Vertex, texCoord)}}};

	}
};

const std::vector<Vertex> vertices = {
    {{-0.5f, -0.5f}, {1.0f, 0.0f, 0.0f}, {1.0f, 0.0f}},
    {{0.5f, -0.5f}, {0.0f, 1.0f, 0.0f}, {0.0f, 0.0f}},
    {{0.5f, 0.5f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}},
    {{-0.5f, 0.5f}, {1.0f, 1.0f, 1.0f}, {1.0f, 1.0f}}};

const std::vector<uint16_t> indices = {
    0, 1, 2,
    2, 3, 0
};

class Engine
{
  public:
	void run() {
		initWindow();
		initVulkan();
		mainLoop();
		cleanup();
	}

  private:
	GLFWwindow *window = nullptr;
	void initWindow() {
		glfwInit();

		// No OpenGL
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		
		// Creating the Window
		window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);

		glfwSetWindowUserPointer(window, this);
		glfwSetFramebufferSizeCallback(window, framebufferResizeCallback);
	}

	static void framebufferResizeCallback(GLFWwindow *window, int width, int height)
	{
		auto app = reinterpret_cast<Engine *>(glfwGetWindowUserPointer(window));
		app->framebufferResized = true;
	}
	
	vk::raii::Context  context;
	vk::raii::Instance instance = nullptr;
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
	vk::raii::SurfaceKHR             surface        = nullptr;
	vk::raii::PhysicalDevice         physicalDevice = nullptr;
	vk::raii::Device                 device         = nullptr;

	vk::raii::DescriptorPool         descriptorPool     = nullptr;
	std::vector<vk::raii::DescriptorSet> descriptorSets;

	vk::raii::Image                  textureImage       = nullptr;
	vk::raii::DeviceMemory           textureImageMemory = nullptr;
	vk::raii::ImageView              textureImageView   = nullptr;
	vk::raii::Sampler                textureSampler     = nullptr;

	vk::raii::Buffer                 vertexBuffer       = nullptr;
	vk::raii::DeviceMemory           vertexBufferMemory = nullptr;

	vk::raii::Buffer                 indexBuffer        = nullptr;
	vk::raii::DeviceMemory           indexBufferMemory  = nullptr;

	std::vector<vk::raii::Buffer>       uniformBuffers;
	std::vector<vk::raii::DeviceMemory> uniformBuffersMemory;
	std::vector<void *>                 uniformBuffersMapped;

	vk::raii::Queue                  graphicsQueue  = nullptr;
	vk::raii::SwapchainKHR           swapChain      = nullptr;
	std::vector<vk::Image>           swapChainImages;
	vk::SurfaceFormatKHR             swapChainSurfaceFormat;
	vk::Extent2D                     swapChainExtent;
	std::vector<vk::raii::ImageView> swapChainImageViews;
	vk::raii::DescriptorSetLayout    descriptorSetLayout = nullptr;
	vk::raii::PipelineLayout         pipelineLayout   = nullptr;
	vk::raii::Pipeline               graphicsPipeline = nullptr;
	vk::raii::CommandPool            commandPool      = nullptr;
	std::vector<vk::raii::CommandBuffer> commandBuffers;
	std::vector<vk::raii::Semaphore>     presentCompleteSemaphores;
	std::vector<vk::raii::Semaphore>     renderFinishedSemaphores;
	std::vector<vk::raii::Fence>         inFlightFences;

	void initVulkan() {
		createInstance();

		setupDebugMessenger();
		
		createSurface();
		
		pickPhysicalDevice();
		createLogicalDevice();
		
		createSwapChain();
		createImageViews();
		
		createDescriptorSetLayout();
		createGraphicsPipeline();
		createCommandPool();

		createTextureImage();
		createTextureImageView();
		createTextureSampler();

		createVertexBuffer();
		createIndexBuffer();
		createUniformBuffers();

		createDescriptorPool();
		createDescriptorSets();

		createCommandBuffers();
		createSyncObjects();
	}

	void createTextureSampler()
	{
		vk::PhysicalDeviceProperties properties = physicalDevice.getProperties();
		vk::SamplerCreateInfo samplerInfo{
			.magFilter        = vk::Filter::eLinear,
			.minFilter        = vk::Filter::eLinear,
			.mipmapMode       = vk::SamplerMipmapMode::eLinear,
			.addressModeU     = vk::SamplerAddressMode::eRepeat,
			.addressModeV     = vk::SamplerAddressMode::eRepeat,
			.addressModeW     = vk::SamplerAddressMode::eRepeat,
			.mipLodBias	  = 0.0f,
			.anisotropyEnable = vk::True,
			.maxAnisotropy    = properties.limits.maxSamplerAnisotropy,
			.compareEnable	  = vk::False,
			.compareOp        = vk::CompareOp::eAlways,
			.minLod		  = 0.0f,
			.maxLod		  = 0.0f,
			.borderColor      = vk::BorderColor::eIntOpaqueBlack,
			.unnormalizedCoordinates = vk::False};

		textureSampler = vk::raii::Sampler(device, samplerInfo);
	}

	vk::raii::ImageView createImageView(vk::Image const &image, vk::Format format)
	{
		vk::ImageViewCreateInfo viewInfo{
		    .image            = image,
		    .viewType         = vk::ImageViewType::e2D,
		    .format           = format,
		    .subresourceRange = {.aspectMask = vk::ImageAspectFlagBits::eColor, .baseMipLevel = 0, .levelCount = 1, .baseArrayLayer = 0, .layerCount = 1}};
		
		return vk::raii::ImageView(device, viewInfo);
	}

	void createTextureImageView()
	{
		textureImageView = createImageView(*textureImage, vk::Format::eR8G8B8A8Srgb);
	}

	vk::raii::CommandBuffer beginSingleTimeCommands()
	{
		vk::CommandBufferAllocateInfo allocInfo{.commandPool = commandPool, .level = vk::CommandBufferLevel::ePrimary, .commandBufferCount = 1};
		vk::raii::CommandBuffer       commandBuffer = std::move(vk::raii::CommandBuffers(device, allocInfo).front());

		vk::CommandBufferBeginInfo beginInfo{.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit};
		commandBuffer.begin(beginInfo);

		return std::move(commandBuffer);
	}

	void endSingleTimeCommands(vk::raii::CommandBuffer &&commandBuffer)
	{
		commandBuffer.end();

		vk::SubmitInfo submitInfo{.commandBufferCount = 1, .pCommandBuffers = &*commandBuffer};
		graphicsQueue.submit(submitInfo, nullptr);
		graphicsQueue.waitIdle();
	}

	std::pair<vk::raii::Image, vk::raii::DeviceMemory> createImage(
	    uint32_t width, uint32_t height, vk::Format format, vk::ImageTiling tiling, vk::ImageUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		vk::ImageCreateInfo imageInfo{.imageType   = vk::ImageType::e2D,
		                              .format      = format,
		                              .extent      = {width, height, 1},
		                              .mipLevels   = 1,
		                              .arrayLayers = 1,
		                              .samples     = vk::SampleCountFlagBits::e1,
		                              .tiling      = tiling,
		                              .usage       = usage,
		                              .sharingMode = vk::SharingMode::eExclusive};

		vk::raii::Image image = vk::raii::Image(device, imageInfo);

		vk::MemoryRequirements memRequirements = image.getMemoryRequirements();
		vk::MemoryAllocateInfo allocInfo{.allocationSize  = memRequirements.size,
		                                 .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};
		vk::raii::DeviceMemory imageMemory = vk::raii::DeviceMemory(device, allocInfo);
		image.bindMemory(imageMemory, 0);

		return {std::move(image), std::move(imageMemory)};
	}

	void createTextureImage()
	{
		int            texWidth, texHeight, texChannels;
		stbi_uc       *pixels    = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
		vk::DeviceSize imageSize = texWidth * texHeight * 4;

		if (!pixels)
		{
			throw std::runtime_error("failed to load texture image!");
		}

		auto [stagingBuffer, stagingBufferMemory] =
		    createBuffer(
			    imageSize,
			    vk::BufferUsageFlagBits::eTransferSrc,
			    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);
		
		void *data = stagingBufferMemory.mapMemory(0, imageSize);
		memcpy(data, pixels, imageSize);
		stagingBufferMemory.unmapMemory();
		
		stbi_image_free(pixels);

		std::tie(textureImage, textureImageMemory) = 
			createImage(texWidth,
				texHeight,
		                vk::Format::eR8G8B8A8Srgb,
		                vk::ImageTiling::eOptimal,
		                vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled,
		                vk::MemoryPropertyFlagBits::eDeviceLocal);

		vk::raii::CommandBuffer commandBuffer = beginSingleTimeCommands();
		transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eUndefined, vk::ImageLayout::eTransferDstOptimal);
		copyBufferToImage(commandBuffer, stagingBuffer, textureImage, static_cast<uint32_t>(texWidth), static_cast<uint32_t>(texHeight));
		transitionImageLayout(commandBuffer, textureImage, vk::ImageLayout::eTransferDstOptimal, vk::ImageLayout::eShaderReadOnlyOptimal);
		endSingleTimeCommands(std::move(commandBuffer));
	}

	void createDescriptorSets()
	{
		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, *descriptorSetLayout);
		vk::DescriptorSetAllocateInfo        allocInfo{
			.descriptorPool     = descriptorPool,
		        .descriptorSetCount = static_cast<uint32_t>(layouts.size()),
		        .pSetLayouts        = layouts.data()};
		
		descriptorSets = device.allocateDescriptorSets(allocInfo);

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DescriptorBufferInfo bufferInfo{.buffer = uniformBuffers[i], .offset = 0, .range = sizeof(UniformBufferObject)};
			vk::DescriptorImageInfo  imageInfo{.sampler = textureSampler, .imageView = textureImageView, .imageLayout = vk::ImageLayout::eShaderReadOnlyOptimal};

			std::array<vk::WriteDescriptorSet, 2> descriptorWrites{{{.dstSet          = descriptorSets[i],
			                                                         .dstBinding      = 0,
			                                                         .dstArrayElement = 0,
			                                                         .descriptorCount = 1,
			                                                         .descriptorType  = vk::DescriptorType::eUniformBuffer,
			                                                         .pBufferInfo     = &bufferInfo},
			                                                        {.dstSet          = descriptorSets[i],
			                                                         .dstBinding      = 1,
			                                                         .dstArrayElement = 0,
			                                                         .descriptorCount = 1,
			                                                         .descriptorType  = vk::DescriptorType::eCombinedImageSampler,
			                                                         .pImageInfo      = &imageInfo}}};
			device.updateDescriptorSets(descriptorWrites, {});
		}
	}

	void createDescriptorPool()
	{
		std::array<vk::DescriptorPoolSize, 2> poolSize{{{.type = vk::DescriptorType::eUniformBuffer, .descriptorCount = MAX_FRAMES_IN_FLIGHT},
		                                                {.type = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = MAX_FRAMES_IN_FLIGHT}}};
		vk::DescriptorPoolCreateInfo          poolInfo{.flags         = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet,
		                                               .maxSets       = MAX_FRAMES_IN_FLIGHT,
		                                               .poolSizeCount = static_cast<uint32_t>(poolSize.size()),
		                                               .pPoolSizes    = poolSize.data()};

		descriptorPool = vk::raii::DescriptorPool(device, poolInfo);
	}

	void createUniformBuffers()
	{
		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
		{
			vk::DeviceSize bufferSize = sizeof(UniformBufferObject);

			auto [buffer, bufferMemory] = createBuffer(
			    bufferSize,
			    vk::BufferUsageFlagBits::eUniformBuffer,
			    vk::MemoryPropertyFlagBits::eHostCoherent | vk::MemoryPropertyFlagBits::eHostVisible);

			uniformBuffers.emplace_back(std::move(buffer));
			uniformBuffersMemory.emplace_back(std::move(bufferMemory));
			uniformBuffersMapped.emplace_back(uniformBuffersMemory.back().mapMemory(0, bufferSize));
		}
	}

	void createDescriptorSetLayout()
	{
		std::array<vk::DescriptorSetLayoutBinding, 2> bindings{
		    {{.binding = 0, .descriptorType = vk::DescriptorType::eUniformBuffer, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eVertex},
		     {.binding = 1, .descriptorType = vk::DescriptorType::eCombinedImageSampler, .descriptorCount = 1, .stageFlags = vk::ShaderStageFlagBits::eFragment}}};

		vk::DescriptorSetLayoutCreateInfo layoutInfo{.bindingCount = static_cast<uint32_t>(bindings.size()), .pBindings = bindings.data()};

		descriptorSetLayout = vk::raii::DescriptorSetLayout(device, layoutInfo);
	}
	
	void createIndexBuffer()
	{
		vk::DeviceSize indexBufferSize = sizeof(indices[0]) * indices.size();
		
		auto [stagingBuffer, stagingBufferMemory] =
		    createBuffer(
		        indexBufferSize,
		        vk::BufferUsageFlagBits::eTransferSrc,
		        vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void *dataStaging = stagingBufferMemory.mapMemory(0, indexBufferSize);
		memcpy(dataStaging, indices.data(), indexBufferSize);
		stagingBufferMemory.unmapMemory();

		std::tie(indexBuffer, indexBufferMemory) =
		    createBuffer(
		        indexBufferSize,
		        vk::BufferUsageFlagBits::eIndexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		        vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer(stagingBuffer, indexBuffer, indexBufferSize);
	}

	uint32_t findMemoryType(uint32_t typeFilter, vk::MemoryPropertyFlags properties)
	{
		vk::PhysicalDeviceMemoryProperties memProperties = physicalDevice.getMemoryProperties();

		for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
		{
			if ((typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & properties) == properties)
			{
				return i;
			}
		}

		throw std::runtime_error("failed to find suitable memory type!");
	}

	std::pair<vk::raii::Buffer, vk::raii::DeviceMemory> createBuffer(vk::DeviceSize size, vk::BufferUsageFlags usage, vk::MemoryPropertyFlags properties)
	{
		vk::BufferCreateInfo bufferCreateInfo{
		    .size        = size,
		    .usage       = usage,
		    .sharingMode = vk::SharingMode::eExclusive};

		vk::raii::Buffer buffer = vk::raii::Buffer(device, bufferCreateInfo);

		vk::MemoryRequirements memRequirements = buffer.getMemoryRequirements();

		vk::MemoryAllocateInfo memoryAllocateInfo{
		    .allocationSize  = memRequirements.size,
		    .memoryTypeIndex = findMemoryType(memRequirements.memoryTypeBits, properties)};

		vk::raii::DeviceMemory bufferMemory = vk::raii::DeviceMemory(device, memoryAllocateInfo);

		buffer.bindMemory(*bufferMemory, 0);

		return {std::move(buffer), std::move(bufferMemory)};
	}

	void copyBuffer(vk::raii::Buffer &srcBuffer, vk::raii::Buffer &dstBuffer, vk::DeviceSize size)
	{
		vk::raii::CommandBuffer commandCopyBuffer = beginSingleTimeCommands();

		commandCopyBuffer.copyBuffer(*srcBuffer, *dstBuffer, vk::BufferCopy{.size = size});
		
		endSingleTimeCommands(std::move(commandCopyBuffer));
	}

	void createVertexBuffer()
	{
		vk::DeviceSize vertexBufferSize = sizeof(vertices[0]) * vertices.size();
		
		auto [stagingBuffer, stagingBufferMemory] = 
			createBuffer(
			    vertexBufferSize,
			    vk::BufferUsageFlagBits::eTransferSrc,
			    vk::MemoryPropertyFlagBits::eHostVisible | vk::MemoryPropertyFlagBits::eHostCoherent);

		void *dataStaging = stagingBufferMemory.mapMemory(0, vertexBufferSize);
		memcpy(dataStaging, vertices.data(), vertexBufferSize);
		stagingBufferMemory.unmapMemory();

		std::tie(vertexBuffer, vertexBufferMemory) =
		    createBuffer(
		        vertexBufferSize,
		        vk::BufferUsageFlagBits::eVertexBuffer | vk::BufferUsageFlagBits::eTransferDst,
		        vk::MemoryPropertyFlagBits::eDeviceLocal);

		copyBuffer(stagingBuffer, vertexBuffer, vertexBufferSize);
	}

	void cleanupSwapChain()
	{
		swapChainImageViews.clear();
		swapChain = nullptr;
	}

	void recreateSwapChain()
	{
		int width = 0, height = 0;
		glfwGetFramebufferSize(window, &width, &height);
		while (width == 0 || height == 0)
		{
			glfwGetFramebufferSize(window, &width, &height);
			glfwWaitEvents();
		}

		device.waitIdle();

		cleanupSwapChain();

		createSwapChain();
		createImageViews();
	}

	void createSyncObjects()
	{
		assert(
		    presentCompleteSemaphores.empty() &&
		    renderFinishedSemaphores.empty() &&
		    inFlightFences.empty());

		for (size_t i = 0; i < swapChainImages.size(); i++) {
			renderFinishedSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
		}

		for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			presentCompleteSemaphores.emplace_back(device, vk::SemaphoreCreateInfo());
			inFlightFences.emplace_back(device, vk::FenceCreateInfo{.flags = vk::FenceCreateFlagBits::eSignaled});
		}
	}

	uint32_t frameIndex = 0;
	void transition_image_layout(
	    uint32_t                imageIndex,
	    vk::ImageLayout         old_layout,
	    vk::ImageLayout         new_layout,
	    vk::AccessFlags2        src_access_mask,
	    vk::AccessFlags2        dst_access_mask,
	    vk::PipelineStageFlags2 src_stage_mask,
	    vk::PipelineStageFlags2 dst_stage_mask)
	{
		vk::ImageMemoryBarrier2 barrier = {
		    .srcStageMask        = src_stage_mask,
		    .srcAccessMask       = src_access_mask,
		    .dstStageMask        = dst_stage_mask,
		    .dstAccessMask       = dst_access_mask,
		    .oldLayout           = old_layout,
		    .newLayout           = new_layout,
		    .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
		    .image               = swapChainImages[imageIndex],
		    .subresourceRange    = {
		           .aspectMask     = vk::ImageAspectFlagBits::eColor,
		           .baseMipLevel   = 0,
		           .levelCount     = 1,
		           .baseArrayLayer = 0,
		           .layerCount     = 1}};

		vk::DependencyInfo dependencyInfo = {
		    .dependencyFlags         = {},
		    .imageMemoryBarrierCount = 1,
		    .pImageMemoryBarriers    = &barrier};
		
		commandBuffers[frameIndex].pipelineBarrier2(dependencyInfo);
	}

	void copyBufferToImage(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Buffer &buffer, vk::raii::Image &image, uint32_t width, uint32_t height)
	{
		vk::BufferImageCopy region{.bufferOffset      = 0,
		                           .bufferRowLength   = 0,
		                           .bufferImageHeight = 0,
		                           .imageSubresource  = {.aspectMask = vk::ImageAspectFlagBits::eColor, .mipLevel = 0, .baseArrayLayer = 0, .layerCount = 1},
		                           .imageOffset       = {0, 0, 0},
		                           .imageExtent       = {width, height, 1}};

		commandBuffer.copyBufferToImage(buffer, image, vk::ImageLayout::eTransferDstOptimal, region);
	}

	void transitionImageLayout(vk::raii::CommandBuffer &commandBuffer, const vk::raii::Image &image, vk::ImageLayout oldLayout, vk::ImageLayout newLayout)
	{
		vk::ImageMemoryBarrier barrier{.oldLayout           = oldLayout,
		                               .newLayout           = newLayout,
		                               .srcQueueFamilyIndex = vk::QueueFamilyIgnored,
		                               .dstQueueFamilyIndex = vk::QueueFamilyIgnored,
		                               .image               = image,
		                               .subresourceRange    = {.aspectMask = vk::ImageAspectFlagBits::eColor, .levelCount = 1, .layerCount = 1}};

		vk::PipelineStageFlags sourceStage;
		vk::PipelineStageFlags destinationStage;

		if (oldLayout == vk::ImageLayout::eUndefined && newLayout == vk::ImageLayout::eTransferDstOptimal)
		{
			barrier.srcAccessMask = {};
			barrier.dstAccessMask = vk::AccessFlagBits::eTransferWrite;

			sourceStage      = vk::PipelineStageFlagBits::eTopOfPipe;
			destinationStage = vk::PipelineStageFlagBits::eTransfer;
		}
		else if (oldLayout == vk::ImageLayout::eTransferDstOptimal && newLayout == vk::ImageLayout::eShaderReadOnlyOptimal)
		{
			barrier.srcAccessMask = vk::AccessFlagBits::eTransferWrite;
			barrier.dstAccessMask = vk::AccessFlagBits::eShaderRead;

			sourceStage      = vk::PipelineStageFlagBits::eTransfer;
			destinationStage = vk::PipelineStageFlagBits::eFragmentShader;
		}
		else
		{
			throw std::invalid_argument("unsupported layout transition!");
		}
		commandBuffer.pipelineBarrier(sourceStage, destinationStage, {}, {}, nullptr, barrier);
	}

	void recordCommandBuffer(uint32_t imageIndex)
	{
		commandBuffers[frameIndex].begin({});

		// Before starting rendering, transition the swapchain image to vk::ImageLayout::eColorAttachmentOptimal
		transition_image_layout(
		    imageIndex,
		    vk::ImageLayout::eUndefined,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    {},                                                        // srcAccessMask (no need to wait for previous operations)
		    vk::AccessFlagBits2::eColorAttachmentWrite,                // dstAccessMask
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput         // dstStage
		);

		vk::ClearValue              clearColor     = vk::ClearColorValue(0.0f, 0.0f, 0.0f, 1.0f);
		
		vk::RenderingAttachmentInfo attachmentInfo = {
		    .imageView   = swapChainImageViews[imageIndex],
		    .imageLayout = vk::ImageLayout::eColorAttachmentOptimal,
		    .loadOp      = vk::AttachmentLoadOp::eClear,
		    .storeOp     = vk::AttachmentStoreOp::eStore,
		    .clearValue  = clearColor};

		vk::RenderingInfo renderingInfo = {
		    .renderArea           = {.offset = {0, 0}, .extent = swapChainExtent},
		    .layerCount           = 1,
		    .colorAttachmentCount = 1,
		    .pColorAttachments    = &attachmentInfo};

		commandBuffers[frameIndex].beginRendering(renderingInfo);

		commandBuffers[frameIndex].bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);
		
		commandBuffers[frameIndex].bindVertexBuffers(0, *vertexBuffer, {0});
		commandBuffers[frameIndex].bindIndexBuffer(*indexBuffer, 0, vk::IndexType::eUint16);
		
		commandBuffers[frameIndex].setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
		commandBuffers[frameIndex].setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

		commandBuffers[frameIndex].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipelineLayout, 0, *descriptorSets[frameIndex], nullptr);

		commandBuffers[frameIndex].drawIndexed(static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);

		commandBuffers[frameIndex].endRendering();

		// After rendering, transition the swapchain image to vk::ImageLayout::ePresentSrcKHR
		transition_image_layout(
		    imageIndex,
		    vk::ImageLayout::eColorAttachmentOptimal,
		    vk::ImageLayout::ePresentSrcKHR,
		    vk::AccessFlagBits2::eColorAttachmentWrite,                // srcAccessMask
		    {},                                                        // dstAccessMask
		    vk::PipelineStageFlagBits2::eColorAttachmentOutput,        // srcStage
		    vk::PipelineStageFlagBits2::eBottomOfPipe                  // dstStage
		);

		commandBuffers[frameIndex].end();
	}

	void createCommandBuffers() {
		vk::CommandBufferAllocateInfo commandBufferAllocInfo{
		    .commandPool        = commandPool,
		    .level              = vk::CommandBufferLevel::ePrimary,
		    .commandBufferCount = MAX_FRAMES_IN_FLIGHT};

		commandBuffers = vk::raii::CommandBuffers(device, commandBufferAllocInfo);
	}

	void createCommandPool() {
		// Litle hack to get the queueIndex back
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
		uint32_t queueIndex = ~0;
		for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++)
		{
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
			    physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface))
			{
				queueIndex = qfpIndex;
				break;
			}
		}
		if (queueIndex == ~0)
		{
			throw std::runtime_error("Unable to find a suitable queue in createLogicalDevice");
		}

		vk::CommandPoolCreateInfo commandPoolCreateInfo{
		    .flags            = vk::CommandPoolCreateFlagBits::eResetCommandBuffer,
		    .queueFamilyIndex = queueIndex};

		commandPool = vk::raii::CommandPool(device, commandPoolCreateInfo);

	}

	[[nodiscard]] vk::raii::ShaderModule createShaderModule(const std::vector<char>& code) const
	{
		vk::ShaderModuleCreateInfo createInfo{
		    .codeSize = code.size() * sizeof(char),
		    .pCode    = reinterpret_cast<const uint32_t *>(code.data())};

		vk::raii::ShaderModule shaderModule(device, createInfo);

		return shaderModule;
	}

	void createGraphicsPipeline() {
		// Shader Module
		auto shaderCode = readFile("slang.spv");
		auto shaderModule = createShaderModule(shaderCode);

		vk::PipelineShaderStageCreateInfo vertexShaderStageInfo{
		    .stage  = vk::ShaderStageFlagBits::eVertex,
		    .module = shaderModule,
		    .pName  = "vertMain"};

		vk::PipelineShaderStageCreateInfo fragmentShaderStageInfo{
		    .stage  = vk::ShaderStageFlagBits::eFragment,
		    .module = shaderModule,
		    .pName  = "fragMain"};

		vk::PipelineShaderStageCreateInfo shaderStages[] = {
		    vertexShaderStageInfo,
		    fragmentShaderStageInfo};

		//Fixed Pipeline Stages
		auto vertexBindingDescription = Vertex::getVertexBindingDescription();
		auto vertexAttributeDescription = Vertex::getVertexAttributeDescription();
		vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo{
		    .vertexBindingDescriptionCount   = 1,
		    .pVertexBindingDescriptions      = &vertexBindingDescription,
		    .vertexAttributeDescriptionCount = static_cast<uint32_t>(vertexAttributeDescription.size()),
		    .pVertexAttributeDescriptions    = vertexAttributeDescription.data()};
		
		vk::PipelineInputAssemblyStateCreateInfo inputeAssemblerInfo{
		    .topology = vk::PrimitiveTopology::eTriangleList};

		std::vector<vk::DynamicState> dynamicStates = {
		    vk::DynamicState::eViewport,
		    vk::DynamicState::eScissor};
		
		vk::PipelineDynamicStateCreateInfo dynamicStateInfo{
		    .dynamicStateCount = static_cast<uint32_t>(dynamicStates.size()),
		    .pDynamicStates    = dynamicStates.data()};

		vk::PipelineViewportStateCreateInfo viewportStateInfo{
		    .viewportCount = 1,
		    .scissorCount  = 1};

		vk::PipelineRasterizationStateCreateInfo rasterizationStateInfo{
		    .depthClampEnable        = vk::False,
		    .rasterizerDiscardEnable = vk::False,
		    .polygonMode             = vk::PolygonMode::eFill,
		    .cullMode                = vk::CullModeFlagBits::eBack,
		    .frontFace               = vk::FrontFace::eCounterClockwise,
		    .depthBiasEnable         = vk::False,
		    .lineWidth               = 1.0f};

		vk::PipelineMultisampleStateCreateInfo multisampleStateInfo{
		    .rasterizationSamples = vk::SampleCountFlagBits::e1,
		    .sampleShadingEnable     = vk::False};
		
		vk::PipelineDepthStencilStateCreateInfo depthStencilStateInfo{};

		vk::PipelineColorBlendAttachmentState colorBlendAttachment{
		    .blendEnable    = vk::False,
		    .colorWriteMask = vk::ColorComponentFlagBits::eR | vk::ColorComponentFlagBits::eG | vk::ColorComponentFlagBits::eB | vk::ColorComponentFlagBits::eA};

		vk::PipelineColorBlendStateCreateInfo colorBlending{
		    .logicOpEnable = vk::False, .logicOp = vk::LogicOp::eCopy, .attachmentCount = 1, .pAttachments = &colorBlendAttachment};

		vk::PipelineLayoutCreateInfo pipelineLayoutCreateInfo{
		    .setLayoutCount         = 1,
		    .pSetLayouts            = &*descriptorSetLayout,
		    .pushConstantRangeCount = 0};

		pipelineLayout = vk::raii::PipelineLayout(device, pipelineLayoutCreateInfo);

		vk::StructureChain<vk::GraphicsPipelineCreateInfo, vk::PipelineRenderingCreateInfo> pipelineCreateInfoChain = {
			{.stageCount          = 2,
		        .pStages             = shaderStages,
		        .pVertexInputState   = &vertexInputStateInfo,
		        .pInputAssemblyState = &inputeAssemblerInfo,
		        .pViewportState      = &viewportStateInfo,
			.pRasterizationState = &rasterizationStateInfo,
		        .pMultisampleState   = &multisampleStateInfo,
		        .pColorBlendState    = &colorBlending,
		        .pDynamicState       = &dynamicStateInfo,
		        .layout              = pipelineLayout,
		        .renderPass          = nullptr},
			{.colorAttachmentCount = 1, .pColorAttachmentFormats = &swapChainSurfaceFormat.format}};

		graphicsPipeline = vk::raii::Pipeline(device, nullptr, pipelineCreateInfoChain.get<vk::GraphicsPipelineCreateInfo>());
	}

	void createImageViews()
	{
		assert(swapChainImageViews.empty());

		swapChainImageViews.reserve(swapChainImages.size());
		for (auto &image : swapChainImages)
		{
			swapChainImageViews.emplace_back(createImageView(image, swapChainSurfaceFormat.format));
		}
	}

	void createSwapChain() {
		auto surfaceCapabilities = physicalDevice.getSurfaceCapabilitiesKHR(*surface);
		swapChainExtent          = chooseSwapChainExtent(surfaceCapabilities);
		uint32_t minImageCount = surfaceCapabilities.minImageCount + 1 > surfaceCapabilities.maxImageCount ? surfaceCapabilities.maxImageCount : surfaceCapabilities.minImageCount + 1;
		
		std::vector<vk::SurfaceFormatKHR> availableFormats      = physicalDevice.getSurfaceFormatsKHR(*surface);
		swapChainSurfaceFormat = chooseSwapChainSurfaceFormat(availableFormats);
		
		std::vector<vk::PresentModeKHR>   availablePresentModes = physicalDevice.getSurfacePresentModesKHR(*surface);

		vk::SwapchainCreateInfoKHR swapChainCreateInfo {
			.surface	  = *surface,

			.minImageCount    = minImageCount,
			.imageFormat      = swapChainSurfaceFormat.format,
			.imageColorSpace  = swapChainSurfaceFormat.colorSpace,
			.imageExtent      = swapChainExtent,
			
			.imageArrayLayers = 1,
			
			.imageUsage       = vk::ImageUsageFlagBits::eColorAttachment,
			.imageSharingMode = vk::SharingMode::eExclusive,
			.preTransform     = surfaceCapabilities.currentTransform,
			.compositeAlpha   = vk::CompositeAlphaFlagBitsKHR::eOpaque,
			.presentMode      = chooseSwapChainPresentMode(availablePresentModes),
			.clipped          = true
		};

		swapChain = vk::raii::SwapchainKHR(device, swapChainCreateInfo);
		swapChainImages = swapChain.getImages();
	}

	void createSurface() {
		VkSurfaceKHR _surface;
		if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
			throw std::runtime_error("Surface creation failed!");
		}

		surface = vk::raii::SurfaceKHR(instance, _surface);
	}

	void createLogicalDevice() {
		// Setup for Graphics Queue
		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();
		uint32_t queueIndex = ~0;
	        for (uint32_t qfpIndex = 0; qfpIndex < queueFamilyProperties.size(); qfpIndex++) {
			if ((queueFamilyProperties[qfpIndex].queueFlags & vk::QueueFlagBits::eGraphics) &&
				physicalDevice.getSurfaceSupportKHR(qfpIndex, *surface)) {
				queueIndex = qfpIndex;
				break;
			}
		}
		if (queueIndex == ~0) {
	            throw std::runtime_error("Unable to find a suitable queue in createLogicalDevice");
		}
		const float queuePriority = 0.5;

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
			.queueFamilyIndex = queueIndex,
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// Features wanted from the queue
		vk::StructureChain<
			vk::PhysicalDeviceFeatures2,
			vk::PhysicalDeviceVulkan13Features,
			vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT,
			vk::PhysicalDeviceVulkan11Features> featureChain = {
		        {.features = {.samplerAnisotropy = true}},
			{.synchronization2 = true, .dynamicRendering = true},
			{.extendedDynamicState = true},
			{.shaderDrawParameters = true}
		};

		const std::array<const char *, 1> requiredDeviceExtensions = {vk::KHRSwapchainExtensionName};

		vk::DeviceCreateInfo deviceCreateInfo {
			.pNext = featureChain.get<vk::PhysicalDeviceFeatures2>(),
			
			.queueCreateInfoCount = 1,
			.pQueueCreateInfos = &deviceQueueCreateInfo,
			
			.enabledExtensionCount = static_cast<uint32_t>(requiredDeviceExtensions.size()),
			.ppEnabledExtensionNames = requiredDeviceExtensions.data()
		};
		
		device = vk::raii::Device(physicalDevice, deviceCreateInfo);
	        graphicsQueue = vk::raii::Queue(device, queueIndex, 0);
	}

	inline vk::SurfaceFormatKHR chooseSwapChainSurfaceFormat(const std::vector<vk::SurfaceFormatKHR>& availableFormats) {
		assert(!availableFormats.empty());

		const auto formatIt = std::ranges::find_if(
			availableFormats,
			[](const auto& format){
			    return format.format == vk::Format::eB8G8R8A8Srgb && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear;
			});
		return formatIt != availableFormats.end() ? *formatIt : availableFormats[0];
	}

	/**
	* @returns always vk::PresentModeKHR::eFifo or fails the assert
	*/
	inline vk::PresentModeKHR chooseSwapChainPresentMode(const std::vector<vk::PresentModeKHR>& availablePresentModes) {
		auto presentModeIt = std::ranges::find_if(
			availablePresentModes,
			[](const auto &presentMode) {
				return presentMode == vk::PresentModeKHR::eFifo;
			});

		assert(presentModeIt != availablePresentModes.end());

		return *presentModeIt;
	}

	inline vk::Extent2D chooseSwapChainExtent(const vk::SurfaceCapabilitiesKHR& capabilities) {
		if (capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
			return capabilities.currentExtent;
		
		int width, height;
		glfwGetFramebufferSize(window, &width, &height);

		return {
		    std::clamp<uint32_t>(width, capabilities.minImageExtent.width, capabilities.maxImageExtent.width),
		    std::clamp<uint32_t>(height, capabilities.minImageExtent.height, capabilities.maxImageExtent.height)
		};
	}

	void pickPhysicalDevice() {
		auto physicalDevices = instance.enumeratePhysicalDevices();

		if (physicalDevices.empty()) {
			throw std::runtime_error("Failed to pick a physical devide!");
		}

		auto physicalDeviceIt = std::ranges::find_if(
			physicalDevices,
			[](const auto &pd) {
				// API version check
				bool supportsVulkan1_3 = pd.getProperties().apiVersion >= vk::ApiVersion13;


				// Dedicated GPU check
		                bool dedicatedGPU = pd.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;


				// Queue family check
				auto queueFamilies    = pd.getQueueFamilyProperties();
				bool supportsGraphics = std::ranges::any_of(
					queueFamilies,
					[](const auto &qpf) {
						return !!(qpf.queueFlags & vk::QueueFlagBits::eGraphics);
					});


				// Required extension check
				const std::array<const char *, 1> requiredDeviceExtensions = {vk::KHRSwapchainExtensionName};
				auto availableDeviceExtensions = pd.enumerateDeviceExtensionProperties();
				bool supportsAllRequiredExtensions = std::ranges::all_of(
					requiredDeviceExtensions,
					[&availableDeviceExtensions](const auto &requiredDeviceExtension) {
						return std::ranges::any_of(
							availableDeviceExtensions,
							[requiredDeviceExtension](const auto &availableDeviceExtension) {
								return strcmp(requiredDeviceExtension, availableDeviceExtension.extensionName) == 0;
						});
					});


				// Feature check
				auto features                 = pd.template getFeatures2<vk::PhysicalDeviceFeatures2,
											 vk::PhysicalDeviceVulkan13Features,
											 vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();

				bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceFeatures2>().features.samplerAnisotropy &&
								features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
								features.template get<vk::PhysicalDeviceVulkan13Features>().synchronization2 &&
								features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;

				return supportsVulkan1_3 && dedicatedGPU && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
			});

		if (physicalDeviceIt == physicalDevices.end()) {
			throw std::runtime_error("Failed to pick a physical devide from the available ones!");
		}

		physicalDevice = *physicalDeviceIt;
	}

	void createInstance() {
		// Engine specific cnfiguration
		constexpr vk::ApplicationInfo appInfo {
			.pApplicationName = "Hello Trig",
			.applicationVersion = VK_MAKE_VERSION(1, 0, 0),
		        .pEngineName        = "Rex Core Engine",
			.engineVersion      = VK_MAKE_VERSION(1, 0, 0),
			.apiVersion         = vk::ApiVersion14
		};

		// Get and check Vulkan Validation layers
		std::vector<char const *> requiredLayers;
		if (enableValidationLayers) {
		    requiredLayers.assign(validationLayers.begin(), validationLayers.end());
		}

		auto layerProperties = context.enumerateInstanceLayerProperties();
		auto unsupportedLayerIt = std::ranges::find_if(
			requiredLayers,
		        [&layerProperties](auto const &requiredLayer) {
				return std::ranges::none_of(
				    layerProperties,
				    [requiredLayer](auto const &layerPropery) {
					return strcmp(layerPropery.layerName, requiredLayer) == 0;
				    });
                        });

		if (unsupportedLayerIt != requiredLayers.end()) {
		    throw std::runtime_error("Required layer not supported:\t" + std::string(*unsupportedLayerIt));
		}

		// Get and check GLFW extensions
	        uint32_t glfwExtensionCount = 0;
		const char **glfwExtensions  = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);
		std::vector  requiredExtensions(glfwExtensions, glfwExtensions + glfwExtensionCount);
		if (enableValidationLayers) {
		    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}

		auto extensionPrperties = context.enumerateInstanceExtensionProperties();
		auto unsupportedPropertyIt = std::ranges::find_if(
			requiredExtensions,
			[&extensionPrperties](auto const& requiredExtension) {
				return std::ranges::none_of(
					extensionPrperties,
					[requiredExtension](auto const& extensionProperty) {
						return strcmp(extensionProperty.extensionName, requiredExtension) == 0;
					});
			});

		if (unsupportedPropertyIt != requiredExtensions.end()){
			throw std::runtime_error("Required GLFW extension not supported: " + std::string(*unsupportedPropertyIt));
		}

		vk::InstanceCreateInfo createInfo {
			.pApplicationInfo	 = &appInfo,

			// Vulkan Validation layers 
		        .enabledLayerCount       = static_cast<uint32_t>(requiredLayers.size()),
		        .ppEnabledLayerNames     = requiredLayers.data(),
		        
			// GLFW extensions
			.enabledExtensionCount   = static_cast<uint32_t>(requiredExtensions.size()),
		        .ppEnabledExtensionNames = requiredExtensions.data()};

		instance = vk::raii::Instance(context, createInfo);
	}
	
	void setupDebugMessenger()
	{
		if (!enableValidationLayers) {
			return;
		}
		
		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eVerbose |
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
		    vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(
		    vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral |
		    vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
		    vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);

		vk::DebugUtilsMessengerCreateInfoEXT debugUtilsMessengerCreateInfoEXT{
		    .messageSeverity = severityFlags,
		    .messageType     = messageTypeFlags,
		    .pfnUserCallback = &debugCallback};
	}
	
	void mainLoop() {
		std::cout << "All works fine!" << std::endl;
		
		while (!glfwWindowShouldClose(window)) {
			glfwPollEvents();
			drawFrame();
		}

		device.waitIdle();
	}

	void updateUniformBuffer(uint32_t currentImage)
	{
		static auto startTime = std::chrono::high_resolution_clock::now();

		auto  currentTime = std::chrono::high_resolution_clock::now();
		float time        = std::chrono::duration<float, std::chrono::seconds::period>(currentTime - startTime).count();

		UniformBufferObject ubo{};
		ubo.model = rotate(glm::mat4(1.0f), time * glm::radians(90.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.view  = lookAt(glm::vec3(2.0f, 2.0f, 2.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 0.0f, 1.0f));
		ubo.proj = glm::perspective(glm::radians(45.0f), static_cast<float>(swapChainExtent.width) / static_cast<float>(swapChainExtent.height), 0.1f, 10.0f);
		ubo.proj[1][1] *= -1;

		memcpy(uniformBuffersMapped[currentImage], &ubo, sizeof(ubo));
	}

	bool framebufferResized = false;
	void drawFrame() {
		auto fenceRes = device.waitForFences(*inFlightFences[frameIndex], vk::True, UINT64_MAX);
		if (fenceRes != vk::Result::eSuccess) {
			throw std::runtime_error("failed to wait for fence!");
		}

		auto [res, imageIndex] = swapChain.acquireNextImage(UINT64_MAX, *presentCompleteSemaphores[frameIndex], nullptr);
		if (res == vk::Result::eErrorOutOfDateKHR)
		{
			recreateSwapChain();
			return;
		}
		if (res != vk::Result::eSuccess && res != vk::Result::eSuboptimalKHR)
		{
			assert(res == vk::Result::eTimeout || res == vk::Result::eNotReady);
			throw std::runtime_error("failed to acquire swap chain image!");
		}
		device.resetFences(*inFlightFences[frameIndex]);

		recordCommandBuffer(imageIndex);

		vk::PipelineStageFlags waitDestinationStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		updateUniformBuffer(frameIndex);

		const vk::SubmitInfo   submitInfo{.waitSemaphoreCount   = 1,
		                                  .pWaitSemaphores      = &*presentCompleteSemaphores[frameIndex],
		                                  .pWaitDstStageMask    = &waitDestinationStageMask,
		                                  .commandBufferCount   = 1,
		                                  .pCommandBuffers      = &*commandBuffers[frameIndex],
		                                  .signalSemaphoreCount = 1,
		                                  .pSignalSemaphores    = &*renderFinishedSemaphores[imageIndex]};
		
		graphicsQueue.submit(submitInfo, *inFlightFences[frameIndex]);

		const vk::PresentInfoKHR presentInfoKHR{
		    .waitSemaphoreCount = 1,
		    .pWaitSemaphores    = &*renderFinishedSemaphores[imageIndex],
		    .swapchainCount     = 1,
		    .pSwapchains        = &*swapChain,
		    .pImageIndices      = &imageIndex};

		auto result = graphicsQueue.presentKHR(presentInfoKHR);
		if (result == vk::Result::eSuboptimalKHR || result == vk::Result::eErrorOutOfDateKHR || framebufferResized)
		{
			framebufferResized = false;
			recreateSwapChain();
		}
		else
		{
			// Code should not hang here in normal use
			assert(result == vk::Result::eSuccess);
		}

		frameIndex = (frameIndex + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void cleanup() {
		cleanupSwapChain();

		glfwDestroyWindow(window);
		glfwTerminate();
	}
};

int main()
{
	try
	{
		Engine app;
		app.run();
	}
	catch (const std::exception &e)
	{
		std::cerr << "EXCEPTION!!!" << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}