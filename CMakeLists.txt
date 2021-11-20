cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

if (TARGET mpgx)
	return()
endif ()

option(MPGX_BUILD_EXAMPLES "Build MPGX usage examples" ON)
option(MPGX_USE_VULKAN "Use modern Vulkan API library" ON)

set(MPGX_LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

if (MPGX_USE_VULKAN)
	list(APPEND MPGX_LANGUAGES CXX)
	set(CMAKE_CXX_STANDARD 11)
	set(CMAKE_CXX_STANDARD_REQUIRED TRUE)
endif ()

project(mpgx 
	VERSION 1.0.0
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

set(MPGX_LINK_LIBRARIES
	cmmt
	freetype
	glad
	glfw
	mpio
	mpmt)

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

configure_file(
	cmake/defines.h.in
	mpgx/defines.h)

add_library(mpgx STATIC
	source/mpgx/pipelines/bloom_pipeline.c
	source/mpgx/pipelines/color_pipeline.c
	source/mpgx/pipelines/diffuse_pipeline.c
	source/mpgx/pipelines/gaussblur_pipeline.c
	source/mpgx/pipelines/gradsky_pipeline.c
    source/mpgx/pipelines/simpshad_pipeline.c
	source/mpgx/pipelines/sprite_pipeline.c
	source/mpgx/pipelines/texcol_pipeline.c
	source/mpgx/pipelines/texspr_pipeline.c

	source/mpgx/renderers/color_renderer.c
	source/mpgx/renderers/diffuse_renderer.c
	source/mpgx/renderers/gradsky_renderer.c
	source/mpgx/renderers/simpshad_renderer.c
	source/mpgx/renderers/sprite_renderer.c
	source/mpgx/renderers/texcol_renderer.c
	source/mpgx/renderers/texspr_renderer.c
	source/mpgx/renderers/text_renderer.c

	source/mpgx/free_camera.c
	source/mpgx/interface.c
	source/mpgx/md5.c
	source/mpgx/renderer.c
	source/mpgx/text.c
	source/mpgx/transformer.c
	source/mpgx/window.c)
target_link_libraries(mpgx PUBLIC
	${MPGX_LINK_LIBRARIES})
target_include_directories(mpgx PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include
	${PROJECT_SOURCE_DIR}/libraries/stb)

if (MPGX_BUILD_EXAMPLES)
	add_executable(mpgx-window-example
		examples/window_example.c)
	target_link_libraries(mpgx-window-example PRIVATE
		mpgx)
	target_include_directories(mpgx-window-example PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	set(WINDOW_EXAMPLE_RESOURCES
		shaders/vulkan/diffuse.frag.spv
		shaders/vulkan/diffuse.vert.spv
		shaders/opengl/diffuse.frag
		shaders/opengl/diffuse.vert)

	foreach (FILE ${WINDOW_EXAMPLE_RESOURCES})
		configure_file(
			resources/${FILE}
			resources/${FILE} COPYONLY)
	endforeach (FILE)
endif ()
