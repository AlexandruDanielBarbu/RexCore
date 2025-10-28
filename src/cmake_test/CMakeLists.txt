cmake_minimum_required(VERSION 3.20)

# set(CMAKE_CXX_COMPILER "C:/mingw64/bin/g++.exe")

project(CMakeTest LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)


include_directories(${CMAKE_SOURCE_DIR}/../../thirdparty/glm)

add_executable(CMakeTest main.cpp)
set_target_properties(CMakeTest PROPERTIES
    WIN32_EXECUTABLE FALSE
)