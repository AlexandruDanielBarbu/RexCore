#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Log.hpp"

VkResult CreateDebugUtilsMessengerEXT(
    VkInstance instance, const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
    const VkAllocationCallbacks* pAllocator,
    VkDebugUtilsMessengerEXT*    pDebugMessenger) {
        auto func = (PFN_vkCreateDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkCreateDebugUtilsMessengerEXT");
        if (func != nullptr) {
                return func(instance, pCreateInfo, pAllocator, pDebugMessenger);
        } else {
                return VK_ERROR_EXTENSION_NOT_PRESENT;
        }
}

void DestroyDebugUtilsMessengerEXT(VkInstance                   instance,
                                   VkDebugUtilsMessengerEXT     debugMessenger,
                                   const VkAllocationCallbacks* pAllocator) {
        auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(
            instance, "vkDestroyDebugUtilsMessengerEXT");
        if (func != nullptr) {
                func(instance, debugMessenger, pAllocator);
        }
}

class HelloTriangle {
      public:
        void run() {
                initWindow();
                initVulkan();
                mainLoop();
                cleanup();
        }

      private:
        const uint32_t WIDTH        = 800;
        const uint32_t HEIGHT       = 800;
        const char*    APP_TITLE    = "RexCore_triangle";
        const char*    ENGINE_TITLE = "RexCore";

        const std::vector<const char*> validationLayers = {
            "VK_LAYER_KHRONOS_validation"};

#ifdef NDEBUG
        const bool enableValidationLayers = false;
#else
        const bool enableValidationLayers = true;
#endif // NDEBUG


        GLFWwindow*              window         = nullptr;
        VkInstance               instance       = {};
        VkPhysicalDevice         physicalDevice = VK_NULL_HANDLE;
        VkDebugUtilsMessengerEXT debugMessenger = {};

        void initWindow() {
                glfwInit();

                // cut opengl out from now on
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

                // forget about resizing for now
                glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

                window = glfwCreateWindow(WIDTH, HEIGHT, APP_TITLE, nullptr,
                                          nullptr);
        }

        void initVulkan() {
                createInstance();
                setupDebugMessenger();
                pickPhysicalDevice();
        }

        void setupDebugMessenger() {
                if (!enableValidationLayers)
                        return;

                VkDebugUtilsMessengerCreateInfoEXT createInfo = {};
                createInfo.sType =
                    VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                createInfo.messageSeverity =
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                createInfo.messageType =
                    VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                    VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                createInfo.pfnUserCallback = debugCallback;
                createInfo.pUserData       = nullptr;

                if (CreateDebugUtilsMessengerEXT(instance, &createInfo, nullptr,
                                                 &debugMessenger) !=
                    VK_SUCCESS) {
                        throw std::runtime_error(
                            "failed to set up debug messenger!");
                }
        }

        bool checkExtensionsSupport(
            const std::vector<VkExtensionProperties>& extensions,
            const std::vector<const char*>&           required) {
                for (const auto& requiredExtension : required) {
                        bool found = false;
                        for (const auto& extension : extensions) {
                                if (std::strcmp(extension.extensionName,
                                                requiredExtension) == 0) {
                                        found = true;
                                        break;
                                }
                        }

                        if (!found)
                                return false;
                }
                return true;
        }

        bool checkValidationLayerSupport() {
                // Get supported layers
                uint32_t extensionCount;
                vkEnumerateInstanceLayerProperties(&extensionCount, nullptr);

                std::vector<VkLayerProperties> layers(extensionCount);
                vkEnumerateInstanceLayerProperties(&extensionCount,
                                                   layers.data());

                // Check if wanted layers are here
                for (const auto& validationLayer : validationLayers) {
                        bool found = false;
                        for (const auto& layer : layers) {
                                if (strcmp(validationLayer, layer.layerName) ==
                                    0) {
                                        found = true;
                                        break;
                                }
                        }

                        if (!found)
                                return false;
                }

                return true;
        }

        const std::vector<VkExtensionProperties>
        getAndPrintAllSupportedExtensions() {
                uint32_t extensionCount;
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                       nullptr);
                std::vector<VkExtensionProperties> extensions(extensionCount);
                vkEnumerateInstanceExtensionProperties(nullptr, &extensionCount,
                                                       extensions.data());

                // Print supported extensions
                for (const auto& extension : extensions) {
                        std::cout << extension.extensionName << '\t'
                                  << extension.specVersion << std::endl;
                }

                return extensions;
        }

        std::vector<const char*> getRequiredExtensions() {
                uint32_t     glfwExtensionCount = 0;
                const char** glfwExtensions     = nullptr;
                glfwExtensions =
                    glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

                std::vector<const char*> extensions(
                    glfwExtensions, glfwExtensions + glfwExtensionCount);

                if (enableValidationLayers) {
                        extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
                }

                return extensions;
        }

        // Vulkan validation layers debug callback for printing messages
        static VKAPI_ATTR VkBool32 VKAPI_CALL
        debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
                      VkDebugUtilsMessageTypeFlagsEXT        messageType,
                      const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
                      void*                                       pUserData) {

                std::cerr << "validation layer: " << pCallbackData->pMessage
                          << std::endl;


                return VK_FALSE;
        }

        void createInstance() {
                // Set up the info of the instance
                VkApplicationInfo appInfo  = {};
                appInfo.sType              = VK_STRUCTURE_TYPE_APPLICATION_INFO;
                appInfo.pApplicationName   = APP_TITLE;
                appInfo.applicationVersion = VK_MAKE_VERSION(1, 0, 0);
                appInfo.pEngineName        = ENGINE_TITLE;
                appInfo.engineVersion      = VK_MAKE_VERSION(1, 0, 0);


                // Use latest API version or default to 1.0
                uint32_t latestVersion = 0;
                if (vkEnumerateInstanceVersion(&latestVersion) != VK_SUCCESS) {
                        latestVersion = VK_API_VERSION_1_0;
                }
                appInfo.apiVersion = latestVersion;


                VkInstanceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;

                // Extensions setup
                auto extensions = getRequiredExtensions();
                if (!checkExtensionsSupport(getAndPrintAllSupportedExtensions(),
                                            extensions)) {
                        throw std::runtime_error(
                            "Some GLFW extensions are not supported!");
                }

                createInfo.enabledExtensionCount =
                    static_cast<uint32_t>(extensions.size());
                createInfo.ppEnabledExtensionNames = extensions.data();

                // Validation layers setup
                if (enableValidationLayers && !checkValidationLayerSupport()) {
                        throw std::runtime_error(
                            "Validation layers not supported!");
                }

                // validation layers for creating validation layers
                VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
                if (enableValidationLayers) {
                        createInfo.enabledLayerCount =
                            static_cast<uint32_t>(validationLayers.size());
                        createInfo.ppEnabledLayerNames =
                            validationLayers.data();

                        debugCreateInfo.sType =
                            VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
                        debugCreateInfo.messageSeverity =
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
                        debugCreateInfo.messageType =
                            VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT |
                            VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
                        debugCreateInfo.pfnUserCallback = debugCallback;
                        debugCreateInfo.pUserData       = nullptr;

                        createInfo.pNext =
                            (VkDebugUtilsMessengerCreateInfoEXT*)&debugCreateInfo;
                } else {
                        createInfo.enabledLayerCount = 0;
                        createInfo.pNext             = nullptr;
                }

                if (vkCreateInstance(&createInfo, nullptr, &instance) !=
                    VK_SUCCESS) {
                        throw std::runtime_error("failed to create instance!");
                }
        }

        bool isDeviceSuitable(VkPhysicalDevice device) {
                VkPhysicalDeviceProperties deviceProperties;
                vkGetPhysicalDeviceProperties(device, &deviceProperties);

                VkPhysicalDeviceFeatures deviceFeatures;
                vkGetPhysicalDeviceFeatures(device, &deviceFeatures);


                return deviceProperties.deviceType ==
                           VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU;
        }

        void pickPhysicalDevice() {
                uint32_t deviceCount = 0;
                vkEnumeratePhysicalDevices(instance, &deviceCount, nullptr);

                if (deviceCount == 0) {
                        throw std::runtime_error(
                            "failed to find GPUs with Vulkan support!");
                }

                std::vector<VkPhysicalDevice> devices(deviceCount);
                vkEnumeratePhysicalDevices(instance, &deviceCount,
                                           devices.data());

                for (const auto& device : devices) {
                        if (isDeviceSuitable(device)) {

                                physicalDevice = device;
                                break;
                        }
                }

                if (physicalDevice == VK_NULL_HANDLE) {
                        throw std::runtime_error(
                            "failed to find a suitable GPU!");
                }
        }


        void mainLoop() {
                while (!glfwWindowShouldClose(window)) {
                        glfwPollEvents();
                }
        }
        void cleanup() {
                if (enableValidationLayers) {
                        DestroyDebugUtilsMessengerEXT(instance, debugMessenger,
                                                      nullptr);
                }

                vkDestroyInstance(instance, nullptr);
                glfwDestroyWindow(window);
                glfwTerminate();
        }
};

int main() {
        HelloTriangle mainApp;
        try {
                mainApp.run();
        } catch (const std::exception& e) {
                REX_ERROR(e.what());
                return EXIT_FAILURE;
        }
        return EXIT_SUCCESS;
}