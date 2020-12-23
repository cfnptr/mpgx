cmake_minimum_required(VERSION 3.10)

if(${CMAKE_SOURCE_DIR} MATCHES ${CMAKE_BINARY_DIR})
	message(FATAL_ERROR "Prevented in-tree build")
endif()

project(mpgx 
	VERSION 1.0.0
	DESCRIPTION "An open-source multi-platform graphics library"
	HOMEPAGE_URL "https://github.com/cfnptr/mpgx"
	LANGUAGES C)

set(CMAKE_C_STANDARD 99)
set(CMAKE_C_STANDARD_REQUIRED TRUE)

configure_file(
	cmake/defines.h.in
	mpgx/defines.h)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)
add_subdirectory(libraries/glfw)

add_subdirectory(libraries/freetype)

add_library(mpgx STATIC
	source/glad/glad.c
	source/mpgx/pipeline.c
	source/mpgx/text.c
	source/mpgx/ui.c
	source/mpgx/window.c)
target_link_libraries(mpgx PUBLIC
	glfw
	freetype)
target_include_directories(mpgx PUBLIC
	${PROJECT_BINARY_DIR}
	${PROJECT_SOURCE_DIR}/include)
