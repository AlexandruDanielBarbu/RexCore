cmake_minimum_required(VERSION 3.20)

project(RexCore LANGUAGES CXX)

# -------------------------------
# C++
# -------------------------------
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# -------------------------------
# Source files
# -------------------------------
file(GLOB_RECURSE SOURCES src/*.cpp)
file(GLOB_RECURSE HEADERS src/*.hpp)

add_executable(${PROJECT_NAME} ${SOURCES} ${HEADERS} "src/Log.cpp")

# -------------------------------
# Third-party includes
# -------------------------------
set(GLM_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glm")
set(GLFW_INCLUDE_DIR "${CMAKE_SOURCE_DIR}/thirdparty/glfw/include")

target_include_directories(${PROJECT_NAME} PRIVATE
    ${GLM_INCLUDE_DIR}
    ${GLFW_INCLUDE_DIR}
)

# -------------------------------
# Prebuilt GLFW library (Windows)
# -------------------------------
set(GLFW_LIBRARY "${CMAKE_SOURCE_DIR}/thirdparty/glfw/build/src/Debug/glfw3.lib")

add_library(GLFW STATIC IMPORTED)
set_target_properties(GLFW PROPERTIES
    IMPORTED_LOCATION "${GLFW_LIBRARY}"
    INTERFACE_INCLUDE_DIRECTORIES "${GLFW_INCLUDE_DIR}"
)

# -------------------------------
# Vulkan
# -------------------------------
find_package(Vulkan REQUIRED)
if(Vulkan_FOUND)
    message(STATUS "Found Vulkan: ${Vulkan_LIBRARY}")
    target_include_directories(${PROJECT_NAME} PRIVATE ${Vulkan_INCLUDE_DIR})
    target_link_libraries(${PROJECT_NAME} PRIVATE ${Vulkan_LIBRARY})
endif()

# -------------------------------
# Linking
# -------------------------------
target_link_libraries(${PROJECT_NAME} PRIVATE GLFW)
