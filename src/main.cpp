#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>
#include <vector>

#include "Log.hpp"

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


        GLFWwindow* window   = nullptr;
        VkInstance  instance = {};

        void initWindow() {
                glfwInit();

                // cut opengl out from now on
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

                // forget about resizing for now
                glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

                window = glfwCreateWindow(WIDTH, HEIGHT, APP_TITLE, nullptr,
                                          nullptr);
        }

        void initVulkan() { createInstance(); }

        bool checkGlfwExtensions(
            const std::vector<VkExtensionProperties>& extensions,
            const char** glfwExtensions, const uint32_t glfwExtensionCount) {
                for (size_t i = 0; i < glfwExtensionCount; i++) {
                        bool found = false;
                        for (const auto& extension : extensions) {
                                if (std::strcmp(extension.extensionName,
                                                glfwExtensions[i]) == 0) {
                                        found = true;
                                        break;
                                }
                        }

                        if (found == false)
                                return false;
                }
                return true;
        }

        void createInstance() {
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

                VkInstanceCreateInfo createInfo = {};
                createInfo.sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
                createInfo.pApplicationInfo = &appInfo;

                uint32_t     glfwExtensionCount;
                const char** glfwExtensions = nullptr;
                glfwExtensions =
                    glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

                REX_LOG(checkGlfwExtensions(extensions, glfwExtensions,
                                            glfwExtensionCount)
                            ? "GLFW extensions supported"
                            : "Some GLFW extensions are not supported");

                createInfo.enabledExtensionCount   = glfwExtensionCount;
                createInfo.ppEnabledExtensionNames = glfwExtensions;
                // Validation layers off for now
                createInfo.enabledLayerCount = 0;

                if (vkCreateInstance(&createInfo, nullptr, &instance) !=
                    VK_SUCCESS) {
                        throw std::runtime_error(
                            "Failed to create Vulkan Instance!");
                }
        }
        void mainLoop() {
                while (!glfwWindowShouldClose(window)) {
                        glfwPollEvents();
                }
        }

        void cleanup() {
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