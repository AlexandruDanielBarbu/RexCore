#define GLFW_INCLUDE_VULKAN
#include <GLFW/glfw3.h>
#include <cstdlib>
#include <iostream>
#include <stdexcept>

#include "Log.hpp"

class HelloTriangle {
      public:
        void run() {
                initVulkan();
                mainLoop();
                cleanup();
        }

      private:
        const uint32_t WIDTH     = 800;
        const uint32_t HEIGHT    = 800;
        const char*    APP_TITLE = "RexCore_triangle";

        GLFWwindow* window = nullptr;

        void initVulkan() {
                glfwInit();

                // cut opengl out from now on
                glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

                // forget about resizing for now
                glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);

                window = glfwCreateWindow(WIDTH, HEIGHT, APP_TITLE, nullptr,
                                          nullptr);
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