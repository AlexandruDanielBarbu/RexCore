# RexCore 🦖🌋

A Vulkan rendering engine written in modern C++, currently in early development.

RexCore is built on the Vulkan 1.3+ feature set, using dynamic rendering, RAII-based Vulkan bindings, and Slang shaders compiled to SPIR-V at build time.

At its current stage it initializes a Vulkan device, creates a swapchain, and **renders a textured 3D model** loaded from an OBJ file, with depth testing and **correct window-resize handling**.

## Index

- [Features](#features)
- [Requirements](#requirements)
- [Building](#building)
  - [1. Clone](#1-clone)
  - [2. Install dependencies](#2-install-dependencies)
  - [3. Configure and build](#3-configure-and-build)
  - [4. Run](#4-run)
  - [5. Package a release](#5-package-a-release)
- [Running the release build](#running-the-release-build)
  - [What you need](#what-you-need)
- [Project layout](#project-layout)
- [Sources](#sources)
- [AI Policy](#ai-policy)
- [License](#license)
- [Map](#map-)

## Features

- **dynamic rendering**
- **OBJ model loading** through tinyobjloader, with vertex deduplication
- **Textured rendering** — staged image upload, image views, anisotropic sampler
- **Depth buffering** with an automatically selected depth format
- **Uniform buffers** and descriptor pool/sets supplying the model-view-projection matrices
- Device-local **vertex and index buffers** filled through staging buffers
- **Slang** shaders compiled to SPIR-V as part of the build
- Double-buffered rendering with frames-in-flight synchronization

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
vcpkg install glfw3 glm tinyobjloader stb --triplet=x64-windows
```

### 3. Configure and build

> [!CAUTION]
> 1. This demo was developed, tested and run on Windows 11 only. Other operating systems are not supported.
> 2. The demo assumes you have a dedicated GPU. If not, you can remove this line of code:
>
> ```c++
>   // Dedicated GPU check
>   bool dedicatedGPU = pd.getProperties().deviceType == vk::PhysicalDeviceType::eDiscreteGpu;
> ```

Point CMake at your vcpkg toolchain file:

```bash
cmake -S . -B out/build/x64-Release -G "Visual Studio 17 2022" -A x64 \
  -DCMAKE_TOOLCHAIN_FILE="C:/path/to/vcpkg/scripts/buildsystems/vcpkg.cmake"

cmake --build out/build/x64-Release --config Release
```

### 4. Run
```bash
cd out/build/x64-Debug
./RexCore.exe
```

The model and texture in use are chosen at the top of `main.cpp` with
`MODEL_PATH` and `TEXTURE_PATH`.

### 5. Package a release

The build produces a self-contained ZIP through CPack:

```bash
cd out/build/x64-Release
cpack -C Release
```

This writes `RexCore-win64.zip` containing `RexCore.exe`, `slang.spv`, the
`assets/` folder, `glfw3.dll`, and the Visual C++ runtime DLLs.

## Running the release build

Grab the latest `.zip` from the [Releases](https://github.com/AlexandruDanielBarbu/RexCore/releases) page, extract it, and
double-click `RexCore.exe`. Keep all the extracted files together in the same
folder — `RexCore.exe` looks for `slang.spv` and the `assets/` folder right next to it.

### What you need

RexCore runs on a stock **Windows 11 (64-bit)** machine as long as you have:

- **Windows 11, 64-bit (x64)**
- **Up-to-date GPU drivers**
- **A Vulkan 1.3+ capable GPU** — anything from the last few years works.

> [!NOTE]
> By default RexCore expects a **dedicated GPU**. If you only have integrated
> graphics, see the [Configure and build](#3-configure-and-build) section for
> the one line to remove.

## Project layout

```
.
├── main.cpp              # Engine entry point and Vulkan setup
├── CMakeLists.txt        # Build configuration, shader compilation, packaging
├── assets/
│   ├── models/           # OBJ meshes
│   ├── textures/         # PNG / JPG textures
│   └── shaders/
│       └── shader.slang  # Vertex + fragment shaders (Slang)
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
  - set up the release packaging (CMake install rules and CPack)

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
- https://docs.vulkan.org/tutorial/latest/08_Loading_models.html
