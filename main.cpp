#define VULKAN_HPP_NO_STRUCT_CONSTRUCTORS
#include <vulkan/vulkan_raii.hpp>

#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>

#include <iostream>  // report and propagate erros
#include <stdexcept> // report and propagate erros
#include <cstdlib>   // EXIT_FAILURE; EXIT_SUCCESS
#include <algorithm> // ranges

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
		auto graphicsQueueFamilyPropertyIt = std::ranges::find_if(
			queueFamilyProperties,
			[](const auto &qfp) {
				return (qfp.queueFlags & vk::QueueFlagBits::eGraphics) != static_cast<vk::QueueFlagBits>(0);
			});
		auto graphicsQueueIndex = std::distance(queueFamilyProperties.begin(), graphicsQueueFamilyPropertyIt);
		const float queuePriority = 0.5;

		vk::DeviceQueueCreateInfo deviceQueueCreateInfo {
			.queueFamilyIndex = static_cast<uint32_t>(graphicsQueueIndex),
			.queueCount = 1,
			.pQueuePriorities = &queuePriority
		};

		// Features wanted from the queue
		vk::StructureChain<vk::PhysicalDeviceFeatures2, vk::PhysicalDeviceVulkan13Features, vk::PhysicalDeviceExtendedDynamicStateFeaturesEXT> featureChain = {
			{},
			{.dynamicRendering = true},
			{.extendedDynamicState = true}
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
	        graphicsQueue = vk::raii::Queue(device, graphicsQueueIndex, 0);
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