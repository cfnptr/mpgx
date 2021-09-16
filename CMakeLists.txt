cmake_minimum_required(VERSION 3.10)

if (${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif ()

project(mpgx 
	VERSION 0.1.0
	DESCRIPTION "Multi-platform graphics library"
	HOMEPAGE_URL "https://github.com/cfnptr/mpgx"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

option(MPGX_BUILD_EXAMPLES "Build MPGX usage examples" ON)
option(MPGX_USE_VULKAN "Use modern Vulkan API library" ON)

add_subdirectory(libraries/cmmt)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

# Removed support of some font formats
set(CMAKE_DISABLE_FIND_PACKAGE_HarfBuzz ON CACHE BOOL "" FORCE)
set(CMAKE_DISABLE_FIND_PACKAGE_BrotliDec ON CACHE BOOL "" FORCE)
add_subdirectory(libraries/freetype)

set(MPGX_LINK_LIBRARIES
	cmmt
	glfw
	freetype)

if (MPGX_USE_VULKAN)
	find_package(Vulkan)

	if (Vulkan_FOUND)
		list(APPEND MPGX_LINK_LIBRARIES
			Vulkan::Vulkan)
		set(MPGX_SUPPORT_VULKAN 1)
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
	source/glad/glad.c

	source/mpgx/pipelines/color_pipeline.c
	source/mpgx/pipelines/diffuse_pipeline.c
	source/mpgx/pipelines/gradsky_pipeline.c
	source/mpgx/pipelines/sprite_pipeline.c
	source/mpgx/pipelines/texcol_pipeline.c
	source/mpgx/pipelines/texspr_pipeline.c

	source/mpgx/renderers/color_renderer.c
	source/mpgx/renderers/diffuse_renderer.c
	source/mpgx/renderers/gradsky_renderer.c
	source/mpgx/renderers/sprite_renderer.c
	source/mpgx/renderers/texcol_renderer.c
	source/mpgx/renderers/texspr_renderer.c
	source/mpgx/renderers/text_renderer.c

    	source/mpgx/free_camera.c
	source/mpgx/interface.c
	source/mpgx/renderer.c
	source/mpgx/text.c
	source/mpgx/transformer.c
    source/mpgx/window.c)
target_link_libraries(mpgx PUBLIC
	${MPGX_LINK_LIBRARIES})
target_include_directories(mpgx PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)

if (MPGX_BUILD_EXAMPLES)
	add_executable(mpgx-window-example
		examples/window_example.c)
	target_link_libraries(mpgx-window-example PRIVATE
		mpgx)
	target_include_directories(mpgx-window-example PRIVATE
		${PROJECT_BINARY_DIR}
		${PROJECT_SOURCE_DIR}/include)

	set(WINDOW_EXAMPLE_RESOURCES
		shaders/opengl/diffuse.frag
		shaders/opengl/diffuse.vert)

	foreach (FILE ${WINDOW_EXAMPLE_RESOURCES})
		configure_file(
			resources/${FILE}
			resources/${FILE} COPYONLY)
	endforeach (FILE)
endif ()
