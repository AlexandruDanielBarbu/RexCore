cmake_minimum_required (VERSION 3.29)
project (RexCore)


# Tools
find_package (glfw3 REQUIRED)
find_package (glm REQUIRED)
find_package (Vulkan 1.4.335 REQUIRED)
find_package (tinyobjloader REQUIRED)
find_package (stb REQUIRED)

find_program(SLANGC_EXECUTABLE slangc HINTS $ENV{VULKAN_SDK}/bin REQUIRED)


# Shader compilation: every .slang file becomes <name>.spv in the build directory
file(GLOB SHADER_SOURCES CONFIGURE_DEPENDS "${CMAKE_CURRENT_SOURCE_DIR}/assets/shaders/*.slang")

set(SHADER_BINARIES "")
foreach(SHADER_SOURCE IN LISTS SHADER_SOURCES)
    cmake_path(GET SHADER_SOURCE STEM SHADER_NAME)
    set(SHADER_BINARY "${CMAKE_CURRENT_BINARY_DIR}/${SHADER_NAME}.spv")

    # No -entry flags: slangc compiles every [shader("...")] entry point in the file
    add_custom_command(
        OUTPUT ${SHADER_BINARY}
        COMMAND ${SLANGC_EXECUTABLE} ${SHADER_SOURCE}
                -target spirv -profile spirv_1_4+spvRayQueryKHR
                -fvk-use-entrypoint-name
                -o ${SHADER_BINARY}
        DEPENDS ${SHADER_SOURCE}
        COMMENT "Compiling Slang: ${SHADER_SOURCE} -> ${SHADER_NAME}.spv"
        VERBATIM
    )

    list(APPEND SHADER_BINARIES ${SHADER_BINARY})
endforeach()

add_custom_target(generate_shaders DEPENDS ${SHADER_BINARIES})

add_subdirectory(third_party/spdlog)

# yaml-cpp: library only (no tests, parse tools, or install rules)
set(YAML_CPP_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_TOOLS OFF CACHE BOOL "" FORCE)
set(YAML_CPP_BUILD_CONTRIB OFF CACHE BOOL "" FORCE)
set(YAML_CPP_INSTALL OFF CACHE BOOL "" FORCE)
# Match RexCore's dynamic MSVC runtime (/MD); a static /MT yaml-cpp causes LNK2038/LNK2005
set(YAML_MSVC_SHARED_RT ON CACHE BOOL "" FORCE)
add_subdirectory(third_party/yaml-cpp-master)

# Application executable
file(GLOB_RECURSE SOURCES src/*.cpp)
add_executable(RexCore ${SOURCES})
target_include_directories(RexCore PRIVATE src)
target_compile_definitions(RexCore PRIVATE REXCORE_ASSETS="${CMAKE_CURRENT_SOURCE_DIR}")

target_link_libraries(RexCore PRIVATE 
    Vulkan::Vulkan 
    glfw 
    glm::glm 
    tinyobjloader::tinyobjloader
    spdlog::spdlog
    yaml-cpp::yaml-cpp
)

target_include_directories(RexCore PRIVATE ${stb_INCLUDE_DIRS})
set_target_properties(RexCore PROPERTIES CXX_STANDARD 20)

add_dependencies(RexCore generate_shaders)


# Release packaging: everything RexCore.exe needs, in one folder.
install(TARGETS RexCore RUNTIME DESTINATION .)
install(FILES ${SHADER_BINARIES} DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/models"   DESTINATION assets)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/assets/textures" DESTINATION assets)

# glfw3.dll (and any other dependency DLLs)
install(FILES $<TARGET_RUNTIME_DLLS:RexCore> DESTINATION .)

# MSVC runtime, so the friend needs no VC++ Redistributable download
set(CMAKE_INSTALL_SYSTEM_RUNTIME_DESTINATION .)
include(InstallRequiredSystemLibraries)

set(CPACK_GENERATOR "ZIP")
set(CPACK_PACKAGE_FILE_NAME "RexCore-win64")
include(CPack)
