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

if (TARGET mpgx-static)
	return()
endif ()

option(MPGX_BUILD_SHARED "Build MPGX shared library" ON)
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

if (MPGX_BUILD_SHARED)
	set(CMAKE_POSITION_INDEPENDENT_CODE ON)
endif ()

add_subdirectory(libraries/cmmt)
add_subdirectory(libraries/glad)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

set(MPMT_BUILD_SHARED OFF CACHE BOOL "" FORCE)
set(MPMT_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(MPMT_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/mpmt)

set(MPGX_SOURCES
	source/md5.c
	source/window.c)

set(MPGX_LINK_LIBRARIES
	cmmt glfw mpmt-static)

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
	include/mpgx/defines.h)

add_library(mpgx-static STATIC
	${MPGX_SOURCES})
set_target_properties(mpgx-static PROPERTIES
	OUTPUT_NAME "mpgx")
target_link_libraries(mpgx-static PUBLIC
	${MPGX_LINK_LIBRARIES})
target_include_directories(mpgx-static PUBLIC
	${PROJECT_BINARY_DIR}/include
	${PROJECT_SOURCE_DIR}/include)

if (MPGX_BUILD_SHARED)
	add_library(mpgx-shared SHARED
		${MPGX_SOURCES})
	set_target_properties(mpgx-shared PROPERTIES
		OUTPUT_NAME "mpgx"
		WINDOWS_EXPORT_ALL_SYMBOLS ON)
	target_link_libraries(mpgx-shared PUBLIC
		${MPGX_LINK_LIBRARIES})
	target_include_directories(mpgx-shared PUBLIC
		${PROJECT_BINARY_DIR}/include
		${PROJECT_SOURCE_DIR}/include)
endif ()
