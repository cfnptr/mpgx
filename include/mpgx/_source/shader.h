// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/opengl.h"

#include "mpgx/md5.h"
#include <string.h>

typedef struct BaseShader_T
{
	Window window;
	ShaderType type;
#ifndef NDEBUG
	uint8_t hash[MD5_BLOCK_SIZE];
#endif
} BaseShader_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkShader_T
{
	Window window;
	ShaderType type;
#ifndef NDEBUG
	uint8_t hash[MD5_BLOCK_SIZE];
#endif
	VkShaderModule handle;
	VkShaderStageFlags stage;
} VkShader_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlShader_T
{
	Window window;
	ShaderType type;
#ifndef NDEBUG
	uint8_t hash[MD5_BLOCK_SIZE];
#endif
	GLuint handle;
} GlShader_T;
#endif
union Shader_T
{
	BaseShader_T base;
#if MPGX_SUPPORT_VULKAN
	VkShader_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlShader_T gl;
#endif
};

#if MPGX_SUPPORT_OPENGL
#define OPENGL_SHADER_HEADER \
	"#version 330 core\n"
#define OPENGL_ES_SHADER_HEADER \
	"#version 300 es\n"         \
	"precision highp float;\n"  \
	"precision highp int;\n"
#endif

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkShader(
	VkDevice device,
	Shader shader)
{
	if (shader == NULL)
		return;

	vkDestroyShaderModule(
		device,
		shader->vk.handle,
		NULL);
	free(shader);
}
inline static Shader createVkShader(
	VkDevice device,
	Window window,
	ShaderType type,
	const void* code,
	size_t size)
{
	Shader shader = calloc(1, sizeof(Shader_T));

	if (shader == NULL)
		return NULL;

	shader->vk.window = window;
	shader->vk.type = type;

	VkShaderModuleCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		size,
		code,
	};

	VkShaderModule handle;

	VkResult vkResult = vkCreateShaderModule(
		device,
		&createInfo,
		NULL,
		&handle);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkShader(
			device,
			shader);
		return NULL;
	}

	shader->vk.handle = handle;

	VkShaderStageFlags stage;

	switch (type)
	{
	default:
		abort();
	case VERTEX_SHADER_TYPE:
		stage = VK_SHADER_STAGE_VERTEX_BIT;
		break;
	case FRAGMENT_SHADER_TYPE:
		stage = VK_SHADER_STAGE_FRAGMENT_BIT;
		break;
	case COMPUTE_SHADER_TYPE:
		stage = VK_SHADER_STAGE_COMPUTE_BIT;
		break;
	case TESSELLATION_CONTROL_SHADER_TYPE:
		stage = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		break;
	case TESSELLATION_EVALUATION_SHADER_TYPE:
		stage = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		break;
	case GEOMETRY_SHADER_TYPE:
		stage = VK_SHADER_STAGE_GEOMETRY_BIT;
		break;
	case RAY_GENERATION_SHADER_TYPE:
		stage = VK_SHADER_STAGE_RAYGEN_BIT_KHR;
		break;
	case RAY_MISS_SHADER_TYPE:
		stage = VK_SHADER_STAGE_MISS_BIT_KHR;
		break;
	case RAY_CLOSEST_HIT_SHADER_TYPE:
		stage = VK_SHADER_STAGE_CLOSEST_HIT_BIT_KHR;
		break;
	}

	shader->vk.stage = stage;
	return shader;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static bool getGlShaderType(
	ShaderType shaderType,
	GLenum* glShaderType)
{
	if (shaderType == VERTEX_SHADER_TYPE)
	{
		*glShaderType = GL_VERTEX_SHADER;
		return true;
	}
	else if (shaderType == FRAGMENT_SHADER_TYPE)
	{
		*glShaderType = GL_FRAGMENT_SHADER;
		return true;
	}
	else if (shaderType == COMPUTE_SHADER_TYPE)
	{
		*glShaderType = GL_COMPUTE_SHADER;
		return true;
	}
	else
	{
		return false;
	}
}

inline static void destroyGlShader(
	Shader shader)
{
	if (shader == NULL)
		return;

	makeWindowContextCurrent(
		shader->gl.window);

	glDeleteShader(shader->gl.handle);
	assertOpenGL();

	free(shader);
}
inline static Shader createGlShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	GraphicsAPI api)
{
	Shader shader = calloc(1, sizeof(Shader_T));

	if (shader == NULL)
		return NULL;

	shader->gl.window = window;
	shader->gl.type = type;

	GLenum glType;

	bool result = getGlShaderType(
		type,
		&glType);

	if (result == false)
	{
		destroyGlShader(shader);
		return NULL;
	}

	const char* sources[2];
	GLint lengths[2];

	if (api == OPENGL_GRAPHICS_API)
	{
		sources[0] = OPENGL_SHADER_HEADER;
		lengths[0] = (GLint)strlen(OPENGL_SHADER_HEADER);
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		sources[0] = OPENGL_ES_SHADER_HEADER;
		lengths[0] = (GLint)strlen(OPENGL_ES_SHADER_HEADER);
	}
	else
	{
		abort();
	}

	sources[1] = (const char*)code;
	lengths[1] = (GLint)size;

	makeWindowContextCurrent(window);

	GLuint handle = glCreateShader(glType);
	shader->gl.handle = handle;

	glShaderSource(
		handle,
		2,
		sources,
		lengths);

	glCompileShader(handle);

	GLint compileStatus;

	glGetShaderiv(
		handle,
		GL_COMPILE_STATUS,
		&compileStatus);

	if (compileStatus == GL_FALSE)
	{
		GLint length = 0;

		glGetShaderiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				destroyGlShader(shader);
				return NULL;
			}

			glGetShaderInfoLog(
				handle,
				length,
				&length,
				(GLchar*)infoLog);

			const char* typeString;

			if (type == VERTEX_SHADER_TYPE)
				typeString = "vertex";
			else if (type == FRAGMENT_SHADER_TYPE)
				typeString = "fragment";
			else
				typeString = "compute";

			printf("OpenGL %s shader compile error:\n%s",
				typeString,
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		destroyGlShader(shader);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlShader(shader);
		return NULL;
	}

	return shader;
}
#endif
