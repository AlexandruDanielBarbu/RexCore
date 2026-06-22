#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>  // report and propagate erros
#include <stdexcept> // report and propagate erros
#include <cstdlib>   // EXIT_FAILURE; EXIT_SUCCESS
#include <algorithm> // ranges
#include <vector>    // vector
#include <limits>    // std::numeric_limits
#include <algorithm> // std::clamp
#include <fstream>   // for shader binary loading

constexpr uint32_t WIDTH          = 800;
constexpr uint32_t HEIGHT         = 800;
constexpr char     WINDOW_TITLE[] = "Rex Core";

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
		
		// Window resize turned off
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

		// Creating the Window
		window = glfwCreateWindow(WIDTH, HEIGHT, WINDOW_TITLE, nullptr, nullptr);
	}
	
	void initVulkan() {
		createInstance();
		setupDebugMessenger();
		createSurface();
		pickPhysicalDevice();
		createLogicalDevice();
		createSwapChain();
		createImageViews();
		createGraphicsPipeline();
		createCommandPool();
		createCommandBuffer();
	}
	
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
		commandBuffer.pipelineBarrier2(dependencyInfo);
	}

	void recordCommandBuffer(uint32_t imageIndex)
	{
		commandBuffer.begin({});

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

		commandBuffer.beginRendering(renderingInfo);

		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, *graphicsPipeline);

		commandBuffer.setViewport(0, vk::Viewport(0.0f, 0.0f, static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height), 0.0f, 1.0f));
		commandBuffer.setScissor(0, vk::Rect2D(vk::Offset2D(0, 0), swapChainExtent));

		commandBuffer.draw(3, 1, 0, 0);

		commandBuffer.endRendering();

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

		commandBuffer.end();
	}

	vk::raii::CommandBuffer commandBuffer = nullptr;
	void createCommandBuffer()
	{
		vk::CommandBufferAllocateInfo commandBufferAllocInfo{
		    .commandPool        = commandPool,
		    .level              = vk::CommandBufferLevel::ePrimary,
		    .commandBufferCount = 1};

		commandBuffer = std::move(vk::raii::CommandBuffers(device, commandBufferAllocInfo).front());

	}

	vk::raii::CommandPool commandPool = nullptr;
	void createCommandPool()
	{
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

		// Litle hack to get the queueIndex back
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

	vk::raii::PipelineLayout pipelineLayout   = nullptr;
	vk::raii::Pipeline       graphicsPipeline = nullptr;
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
		vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;
		
		vk::PipelineInputAssemblyStateCreateInfo inputeAssemblerInfo{
		    .topology = vk::PrimitiveTopology::eTriangleList};

		vk::Viewport viewport{
			0.0f, 0.0f,
			static_cast<float>(swapChainExtent.width), static_cast<float>(swapChainExtent.height),
		        0.0f, 1.0f};
		
		vk::Rect2D scissor{
		    vk::Offset2D{0, 0},
		    swapChainExtent};
		
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
		    .frontFace               = vk::FrontFace::eClockwise,
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
		    .setLayoutCount         = 0,
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

	std::vector<vk::raii::ImageView> swapChainImageViews;
	void createImageViews() {
		assert(swapChainImageViews.empty());

		vk::ImageViewCreateInfo imageViewCreateInfo {
			.viewType = vk::ImageViewType::e2D,
			.format   = swapChainSurfaceFormat.format,
			.components = {vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity, vk::ComponentSwizzle::eIdentity},
			.subresourceRange = {vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1}
		};

		for (const auto &image : swapChainImages) {
			imageViewCreateInfo.image = image;
			swapChainImageViews.emplace_back(device, imageViewCreateInfo);
		}
	}

	vk::raii::SwapchainKHR swapChain = nullptr;
	std::vector<vk::Image> swapChainImages;
	vk::SurfaceFormatKHR   swapChainSurfaceFormat;
	vk::Extent2D           swapChainExtent;
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

	vk::raii::SurfaceKHR surface = nullptr;
	void createSurface() {
		VkSurfaceKHR _surface;
		if (glfwCreateWindowSurface(*instance, window, nullptr, &_surface) != 0) {
			throw std::runtime_error("Surface creation failed!");
		}

		surface = vk::raii::SurfaceKHR(instance, _surface);
	}

	vk::raii::Device device = nullptr;
	vk::raii::Queue  graphicsQueue = nullptr;
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
		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT, vk::PhysicalDeviceVulkan11Features> featureChain = {
			{},
			{.dynamicRendering = true},
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

	vk::raii::PhysicalDevice physicalDevice = nullptr;
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
				auto features = pd.template getFeatures2<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>();
				bool supportsRequiredFeatures = features.template get<vk::PhysicalDeviceVulkan13Features>().dynamicRendering &&
					features.template get<vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT>().extendedDynamicState;
				
				return supportsVulkan1_3 && dedicatedGPU && supportsGraphics && supportsAllRequiredExtensions && supportsRequiredFeatures;
			});

		if (physicalDeviceIt == physicalDevices.end()) {
			throw std::runtime_error("Failed to pick a physical devide from the available ones!");
		}

		physicalDevice = *physicalDeviceIt;
	}

	vk::raii::Context context;
	vk::raii::Instance instance = nullptr;
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
		        .ppEnabledExtensionNames = requiredExtensions.data()
		};

		instance = vk::raii::Instance(context, createInfo);
	}
	
	vk::raii::DebugUtilsMessengerEXT debugMessenger = nullptr;
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
		}
	}
	void cleanup() {
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
		std::cerr << e.what() << std::endl;
		return EXIT_FAILURE;
	}

	return EXIT_SUCCESS;
}