cmake_minimum_required(VERSION 3.20)

project(CMakeTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------
# Third-party includes
# -------------------------------
set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../thirdparty/glm")
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/../../thirdparty/glfw/include")

include_directories(${GLM_INCLUDE_DIR})
include_directories(${GLFW_INCLUDE_DIR})

# -------------------------------
# Prebuilt GLFW library
# -------------------------------
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/../../thirdparty/glfw/build/src/Debug/glfw3.lib")

add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
)

# -------------------------------
# Vulkan
# -------------------------------
set(VULKAN_INCLUDE_DIR "$ENV{VULKAN_SDK}/Include")
set(VULKAN_LIBRARY "$ENV{VULKAN_SDK}/Lib/vulkan-1.lib")

include_directories(${VULKAN_INCLUDE_DIR})

# -------------------------------
# Executable
# -------------------------------
add_executable(CMakeTest main.cpp)

target_link_libraries(CMakeTest PRIVATE GLFW)
target_link_libraries(CMakeTest PRIVATE ${VULKAN_LIBRARY})


# -------------------------------
# Optional: Windows-specific settings
# -------------------------------
# set_target_properties(CMakeTest PROPERTIES
#     WIN32_EXECUTABLE FALSE
# )