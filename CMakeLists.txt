cmake_minimum_required (VERSION 3.29)
project (RexCore)


# Tools
find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (Vulkan 1.4.335 REQUIRED)
find_package (tinyobjloader REQUIRED)
find_package (stb REQUIRED)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)


# Shader compilation
set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/shader.slang")
set(SHADER_BINARY "${CMAKE_CURRENT_BINARY_DIR}/slang.spv")

add_custom_command(
    OUTPUT ${SHADER_BINARY}
    COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCE} 
            -target spirv -profile spirv_1_4+spvRayQueryKHR 
            -fvk-use-entrypoint-name -entry vertMain -entry fragMain
            -o ${SHADER_BINARY}
    DEPENDS ${SHADER_SOURCE}
    COMMENT "Compiling Slang: ${SHADER_SOURCE}"
    VERBATIM
)

add_custom_target(generate_shaders DEPENDS ${SHADER_BINARY})


# Application executable
add_executable(RexCore main.cpp)
target_compile_definitions(RexCore PRIVATE REXCORE_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(RexCore PRIVATE 
    Vulkan::Vulkan 
    glfw 
    glm::glm 
    tinyobjloader::tinyobjloader
)

target_include_directories(RexCore PRIVATE ${stb_INCLUDE_DIRS})
set_target_properties(RexCore PROPERTIES CXX_STANDARD 20)

add_dependencies(RexCore generate_shaders)
