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

const std::array<char const *, 1> validationLayers = {
    "VK_LAYER_KHRONOS_validation"
};

#ifdef NDEBUG
constexpr bool enableValidationLayers = false;
#else
constexpr bool enableValidationLayers = true;
#endif        // NDEBUG

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
	GLFWwindow *window;
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
		/*if (enableValidationLayers) {
		    requiredExtensions.push_back(vk::EXTDebugUtilsExtensionName);
		}*/

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