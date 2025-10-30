#include <iostream>
#include <glm/glm.hpp>
#include <GLFW/glfw3.h>

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW\n";
        return -1;
    }
    
    glm::vec3 v(1.0f, 1.0f, 1.0f);
    std::cout << "Hello, CMake!" << std::endl;
    std::cout << v.x;
    return 0;
}