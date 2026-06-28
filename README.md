# RexCore 🦖🌋

A Vulkan rendering engine written in modern C++, currently in early development.

RexCore is built on the Vulkan 1.3+ feature set, using dynamic rendering, RAII-based Vulkan bindings (`vulkan_raii.hpp`), and Slang shaders compiled to SPIR-V at build time.

At its current stage it initializes a Vulkan device, creates a swapchain, and **renders a triangle** with **correct window-resize handling**.

<!-- > Development log: https://alexandrudanielbarbu.github.io/RexCore-engine-devlog/ -->

This version (`hello-triangle` branch) contains all the vulkan setup code needed to render a triangle on the screen.

## Index

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
  - [1. Clone](#1-clone)
  - [2. Install dependencies](#2-install-dependencies)
  - [3. Configure and build](#3-configure-and-build)
  - [4. Run](#4-run)
- [Project layout](#project-layout)
- [Sources](#sources)
- [AI Policy](#ai-policy)
- [License](#license)

## Features

- Vulkan 1.3 **dynamic rendering**
- **RAII** Vulkan resource management via `vk::raii`
- **GLFW** windowing with live swapchain recreation on resize
- **Slang** shaders compiled to SPIR-V as part of the build
- Physical-device selection with feature and extension checks (discrete GPU,
  `synchronization2`, `dynamicRendering`, `extendedDynamicState`)
- Double-buffered rendering with frames-in-flight synchronization
- Validation layers and debug messenger enabled in debug builds

## Requirements

| Dependency        | Version            | Notes                                         |
| ----------------- | ------------------ | --------------------------------------------- |
| Vulkan SDK        | ≥ 1.4.335          | Provides headers, `vulkan_raii.hpp`, validation layers, and `slangc` |
| CMake             | ≥ 3.29             |                                               |
| C++ compiler      | C++20              | Tested with MSVC (x64)                        |
| vcpkg             | any recent         | Used to provide the libraries below           |

Libraries (resolved through vcpkg):

- **GLFW3** — windowing and surface creation
- **GLM** — math
- **tinyobjloader** — mesh loading
- **KTX** — texture container loading
- **stb** — image loading

## Building

The project uses CMake with a vcpkg toolchain. The Slang shader is compiled to SPIR-V automatically during the build via `slangc` from the Vulkan SDK.

### 1. Clone

```bash
git clone git@github.com:AlexandruDanielBarbu/RexCore.git
cd RexCore
```

### 2. Install dependencies

```bash
vcpkg install glfw3 glm tinyobjloader ktx stb --triplet=x64-windows
```

### 3. Configure and build

> [!CAUTION]
> 1. This demo was developed, tested and run on Windows 11 only. Other operating systems are not supported.
> 2. The demo assumes you have a dedicated GPU. If not, you can remove this line of code:
```c++
  // Dedicated GPU check
  bool dedicatedGPU = pd.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
```

Point CMake at your vcpkg toolchain file:

```bash
cmake -S . -B build -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake --build build --config Debug
```

### 4. Run

The executable loads the compiled shader (`slang.spv`) from its working
directory, so run it from the build folder:

```bash
cd out/build/x64-Debug
./main
```

## Project layout

```
.
├── main.cpp              # Engine entry point and Vulkan setup
├── CMakeLists.txt        # Build configuration + shader compilation step
├── CMake/                # Custom find-modules for dependencies
├── shaders/
│   └── shader.slang      # Vertex + fragment shaders (Slang)
└── LICENSE
```

## Sources

Reference material used while building RexCore:

- [Vulkan Documentation](https://docs.vulkan.org/spec/latest/index.html)
- [Vulkan Tutorial](https://docs.vulkan.org/tutorial/latest/00_Introduction.html)

## AI Policy

The AI tool used is Claude Pro, with a mix of browser and cmd versions of it.

The uses of AI so far were:
  - restructure the CMake build file
  - explain concepts such as framebuffers, swapchains, semaphores and fences
  - debugging **(!! only if the problem persists after a genuine effort on my end of solving the bug!!)**

## License

Released under the MIT License. See [LICENSE](LICENSE).

