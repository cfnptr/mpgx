cmake_minimum_required(VERSION 3.10)

if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif()

project(mpgx 
	VERSION 1.0.0
	DESCRIPTION "Multi-platform graphics library"
	HOMEPAGE_URL "https://github.com/cfnptr/mpgx"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

# Removed support of some font formats
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON CACHE BOOL "" FORCE)
add_subdirectory(libraries/freetype)

find_package(Vulkan)

set(MPGX_LINK_LIBRARIES
	glfw
	freetype)

if (Vulkan_FOUND)
	set(MPGX_VULKAN_SUPPORT 1)

	list(APPEND MPGX_LINK_LIBRARIES
		Vulkan::Vulkan)
else()
	set(MPGX_VULKAN_SUPPORT 0)
endif()

configure_file(
	cmake/defines.h.in
	mpgx/defines.h)

add_library(mpgx STATIC
	source/glad/glad.c
	source/mpgx/pipeline.c
	source/mpgx/renderer.c
	source/mpgx/text.c
	source/mpgx/transformer.c
    source/mpgx/ui.c
    source/mpgx/window.c)
target_link_libraries(mpgx PUBLIC
	${MPGX_LINK_LIBRARIES})
target_include_directories(mpgx PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)
