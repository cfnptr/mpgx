# Work in Progress!

## Features
* Vulkan API and OpenGL rendering backends
* Window creation, manipulation (using GLFW)
* Optimized text rendering (using FreeType)
* Color, diffuse, sprite, texture, sky shaders

## Supported operating systems
* Ubuntu
* MacOS
* Windows

## Build requirements
* C99 compiler
* [CMake 3.10+](https://cmake.org/)
* [X11](https://www.x.org/) (Linux only)

### Vulkan API support (Optional)
* C++11 compiler
* [Vulkan SDK 1.2+](https://vulkan.lunarg.com/)

## X11 installation
* Ubuntu: sudo apt install xorg-dev

## Cloning
```
git clone https://github.com/cfnptr/mpgx
cd mpgx
git submodule update --init --recursive
```

## Building
```
cmake -DCMAKE_BUILD_TYPE=Release -S . -B build/
cmake --build build/
```

### CMake options
| Name                | Description                   | Default value |
| ------------------- | ----------------------------- | ------------- |
| MPGX_BUILD_EXAMPLES | Build MPGX usage examples     | ON            |
| MPGX_USE_VULKAN     | Use modern Vulkan API library | ON            |

## Third-party
* [cmmt](https://github.com/cfnptr/cmmt/) (BSD-3-Clause License)
* [freetype](https://www.freetype.org/) (FreeType License)
* [glad](https://glad.dav1d.de/) (MIT License)
* [glfw](https://www.glfw.org/) (Zlib License)
* [stb_image](https://nothings.org/) (MIT License)
* [VulkanMemoryAllocator](https://gpuopen.com/vulkan-memory-allocator/) (MIT License)
