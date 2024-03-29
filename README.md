# Successor ----> [Garden](https://github.com/cfnptr/garden)

# MPGX ![CI](https://github.com/cfnptr/mpgx/actions/workflows/cmake.yml/badge.svg)

A library providing generic interface for realtime **graphics** rendering across different platforms.

![Rendering Demo](https://user-images.githubusercontent.com/20398717/174491973-aa39074d-587f-4b2f-8ed1-e4231f47c102.jpeg)

## Features

* Vulkan API and OpenGL rendering backends
* Window creation, manipulation (using GLFW)

## Supported operating systems

* Ubuntu
* MacOS
* Windows

## Build requirements

* C99 compiler
* [Git 2.30+](https://git-scm.com/)
* [CMake 3.10+](https://cmake.org/)
* [X11](https://www.x.org/) (Linux only)

### Vulkan API support (Optional)

* C++11 compiler
* [Vulkan SDK 1.2+](https://vulkan.lunarg.com/)

**macOS**: select ```System global installation``` during setup.

### X11 installation

* Ubuntu: sudo apt install xorg-dev

### CMake options

| Name              | Description                   | Default value |
|-------------------|-------------------------------|---------------|
| MPGX_BUILD_SHARED | Build MPGX shared library     | `ON`          |
| MPGX_USE_VULKAN   | Use modern Vulkan API library | `ON`          |
| MPGX_USE_OPENGL   | Use legacy OpenGL library     | `ON`          |

## Cloning

```
git clone --recursive https://github.com/cfnptr/mpgx
```

## Third-party

* [cmmt](https://github.com/cfnptr/cmmt/) (Apache-2.0 License)
* [glad](https://glad.dav1d.de/) (MIT License)
* [glfw](https://www.glfw.org/) (Zlib License)
* [mpmt](https://github.com/cfnptr/mpmt/) (Apache-2.0 License)
* [VulkanMemoryAllocator](https://gpuopen.com/vulkan-memory-allocator/) (MIT License)

## Examples

You can find usage examples in source code of the [Uran](https://github.com/cfnptr/uran/) library.
