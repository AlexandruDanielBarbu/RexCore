cmake_minimum_required (VERSION 3.29)

project (RexCore)

# TOOLS & LIBRARIES
find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (Vulkan 1.4.335 REQUIRED)      # Require Vulkan SDK version 1.4.335 or higher
find_package (tinyobjloader REQUIRED)
# find_package (tinygltf REQUIRED)
find_package (KTX REQUIRED)
find_package (stb REQUIRED)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)

set(SHADER_SOURCE "${CMAKE_CURRENT_SOURCE_DIR}/shaders/shader.slang")
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
# Create a "target" for the shader so we can link it to our app
add_custom_target(generate_shaders DEPENDS ${SHADER_BINARY})

# 3. EXECUTABLE SETUP
add_executable(main main.cpp)

target_link_libraries(main PRIVATE 
    Vulkan::Vulkan 
    glfw 
    glm::glm 
    tinyobjloader::tinyobjloader
)

target_include_directories(main PRIVATE ${stb_INCLUDE_DIRS})
set_target_properties(main PROPERTIES CXX_STANDARD 20)

# 4. THE BOND
# This line ensures the shader compiles BEFORE the C++ code starts
add_dependencies(main generate_shaders)