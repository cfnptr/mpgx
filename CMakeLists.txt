# Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#     http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License.

cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET mpgx)
	return()
endif ()

option(MPGX_BUILD_EXAMPLES "Build MPGX usage examples" ON)
option(MPGX_USE_VULKAN "Use modern Vulkan API library" ON)
option(MPGX_USE_OPENGL "Use legacy OpenGL library" ON)

set(MPGX_LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

if (MPGX_USE_VULKAN)
	list(APPEND MPGX_LANGUAGES CXX)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif ()

project(mpgx 
	VERSION 0.0.1
	DESCRIPTION "Multi-platform graphics library"
	HOMEPAGE_URL "https://github.com/cfnptr/mpgx"
	LANGUAGES ${MPGX_LANGUAGES})

add_subdirectory(libraries/cmmt)
add_subdirectory(libraries/glad)
add_subdirectory(libraries/mpio)

set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE BOOL "" FORCE)
add_subdirectory(libraries/freetype)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

set(MPMT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpmt)

set(MPGX_SOURCES
	source/mpgx/pipelines/bloom_pipeline.c
	source/mpgx/pipelines/color_pipeline.c
	source/mpgx/pipelines/diffuse_pipeline.c
    source/mpgx/pipelines/gaussian_blur_pipeline.c
	source/mpgx/pipelines/gradient_sky_pipeline.c
	source/mpgx/pipelines/ray_tracing_color_pipeline.c
	source/mpgx/pipelines/simple_shadow_pipeline.c
	source/mpgx/pipelines/sprite_pipeline.c
	source/mpgx/pipelines/texture_color_pipeline.c
	source/mpgx/pipelines/texture_sprite_pipeline.c

	source/mpgx/renderers/color_renderer.c
	source/mpgx/renderers/diffuse_renderer.c
	source/mpgx/renderers/simple_shadow_renderer.c
	source/mpgx/renderers/sprite_renderer.c
	source/mpgx/renderers/texture_color_renderer.c
	source/mpgx/renderers/texture_sprite_renderer.c
	source/mpgx/renderers/text_renderer.c

	source/mpgx/free_camera.c
	source/mpgx/graphics_renderer.c
	source/mpgx/interface.c
	source/mpgx/md5.c
	source/mpgx/text.c
	source/mpgx/transformer.c
	source/mpgx/window.c)

set(MPGX_LINK_LIBRARIES
	cmmt freetype glfw mpio mpmt)

if (MPGX_USE_VULKAN)
	find_package(Vulkan)

	if (Vulkan_FOUND)
		set(MPGX_SUPPORT_VULKAN 1)
		add_subdirectory(libraries/VulkanMemoryAllocator)

		list(APPEND MPGX_LINK_LIBRARIES
			Vulkan::Vulkan
			VulkanMemoryAllocator)
	else ()
		set(MPGX_SUPPORT_VULKAN 0)
	endif ()
else ()
	set(MPGX_SUPPORT_VULKAN 0)
endif ()

if (MPGX_USE_OPENGL)
	set(MPGX_SUPPORT_OPENGL 1)
	list(APPEND MPGX_LINK_LIBRARIES glad)
else ()
	set(MPGX_SUPPORT_OPENGL 0)
endif ()

configure_file(
	cmake/defines.h.in
	mpgx/defines.h)

add_library(mpgx-static STATIC
	${MPGX_SOURCES})
set_target_properties(mpgx-static PROPERTIES
	OUTPUT_NAME "mpgx")
target_link_libraries(mpgx-static PUBLIC
	${MPGX_LINK_LIBRARIES})
target_include_directories(mpgx-static PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/libraries/stb)

if (MPGX_BUILD_EXAMPLES)
	add_executable(mpgx-window-example
		examples/window_example.c)
	target_link_libraries(mpgx-window-example PRIVATE
		mpgx-static)
	target_include_directories(mpgx-window-example PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	add_executable(mpgx-ray-tracing-example
		examples/ray_tracing_example.c)
	target_link_libraries(mpgx-ray-tracing-example PRIVATE
		mpgx-static)
	target_include_directories(mpgx-ray-tracing-example PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	set(EXAMPLE_RESOURCES
		shaders/vulkan/diffuse.frag.spv
		shaders/vulkan/diffuse.vert.spv
		shaders/opengl/diffuse.frag
		shaders/opengl/diffuse.vert

		shaders/vulkan/color.rchit.spv
		shaders/vulkan/color.rgen.spv
		shaders/vulkan/color.rmiss.spv)

	foreach (FILE ${EXAMPLE_RESOURCES})
		configure_file(
			resources/${FILE}
			resources/${FILE} COPYONLY)
	endforeach (FILE)
endif ()
