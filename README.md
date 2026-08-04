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
cmake -S . -B build -G "Visual Studio 17 2022" -A x64 \
  -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake --build build --config Release
```

### 4. Run

The executable loads the compiled shader (`slang.spv`) from its working
directory, so run it from the build folder:

```bash
cd out/build/x64-Debug
./main
```

## Running the release build

Grab the latest `.zip` from the [Releases](https://github.com/AlexandruDanielBarbu/RexCore/releases/tag/v1.0.0) page, extract it, and
double-click `main.exe`. Keep all the extracted files together in the same
folder — `main.exe` looks for `slang.spv` right next to it.

### What you need

RexCore runs on a stock **Windows 11 (64-bit)** machine as long as you have:

- **Windows 11, 64-bit (x64)** — the build is x64 only.
- 🎮 **Up-to-date GPU drivers** — grab the latest from your GPU vendor
  These ship the Vulkan runtime (`vulkan-1.dll`) the engine relies on.
- 🖥️ **A Vulkan 1.3+ capable GPU** — anything from the last few years works.
- 📦 **Microsoft Visual C++ Redistributable (x64)** — most systems already have
  it. If launching gives a `VCRUNTIME140.dll was not found` error, install it and try again.

> [!NOTE]
> By default RexCore expects a **dedicated GPU**. If you only have integrated
> graphics, see the [Configure and build](#3-configure-and-build) section for
> the one line to remove.

### You do **not** need

- ❌ The Vulkan SDK — that's for developers; validation layers are off in release.
- ❌ CMake, vcpkg, or a C++ compiler — those build the engine; you're just running it.
- ❌ Slang — the shader is already compiled into `slang.spv` for you.

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

## Map 🗺️📍

- https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/01_Vertex_buffer_creation.html
- https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/02_Staging_buffer.html
- https://docs.vulkan.org/tutorial/latest/04_Vertex_buffers/03_Index_buffer.html
- https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/00_Descriptor_set_layout_and_buffer.html
- https://docs.vulkan.org/tutorial/latest/05_Uniform_buffers/01_Descriptor_pool_and_sets.html
- https://docs.vulkan.org/tutorial/latest/06_Texture_mapping/00_Images.html#_layout_transitions
- https://docs.vulkan.org/tutorial/latest/06_Texture_mapping/00_Images.html#_layout_transitions
- https://docs.vulkan.org/tutorial/latest/06_Texture_mapping/01_Image_view_and_sampler.html
- https://docs.vulkan.org/tutorial/latest/06_Texture_mapping/02_Combined_image_sampler.html
- https://docs.vulkan.org/tutorial/latest/07_Depth_buffering.html
