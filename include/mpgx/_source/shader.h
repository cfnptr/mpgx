// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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
	VkShaderStageFlags stage;
	VkShaderModule handle;
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
	assert(device != NULL);

	if (shader == NULL)
		return;

	vkDestroyShaderModule(
		device,
		shader->vk.handle,
		NULL);
	free(shader);
}
inline static MpgxResult createVkShader(
	VkDevice device,
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	Shader* shader)
{
	assert(device != NULL);
	assert(window != NULL);
	assert(type < SHADER_TYPE_COUNT);
	assert(code != NULL);
	assert(size != 0);
	assert(shader != NULL);

	Shader shaderInstance = calloc(1, sizeof(Shader_T));

	if (shaderInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	shaderInstance->vk.window = window;
	shaderInstance->vk.type = type;

	VkShaderStageFlags stage;

	switch (type)
	{
	default:
		destroyVkShader(
			device,
			shaderInstance);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
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

	shaderInstance->vk.stage = stage;

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
			shaderInstance);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_INVALID_SHADER_NV)
			return BAD_SHADER_CODE_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	shaderInstance->vk.handle = handle;

	*shader = shaderInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static bool getGlShaderType(
	ShaderType shaderType,
	GLenum* glShaderType)
{
	assert(shaderType < SHADER_TYPE_COUNT);
	assert(glShaderType != NULL);

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
inline static MpgxResult createGlShader(
	Window window,
	ShaderType type,
	const void* code,
	size_t size,
	GraphicsAPI api,
	Shader* shader)
{
	assert(window != NULL);
	assert(type < SHADER_TYPE_COUNT);
	assert(code != NULL);
	assert(size != 0);
	assert(api < GRAPHICS_API_COUNT);
	assert(shader != NULL);

	Shader shaderInstance = calloc(1, sizeof(Shader_T));

	if (shaderInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	shaderInstance->gl.window = window;
	shaderInstance->gl.type = type;

	GLenum glType;

	bool result = getGlShaderType(
		type,
		&glType);

	if (result == false)
	{
		destroyGlShader(shaderInstance);
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
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
	shaderInstance->gl.handle = handle;

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
				destroyGlShader(shaderInstance);
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
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
				abort();

			printf("OpenGL %s shader compile error:\n%s",
				typeString,
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		destroyGlShader(shaderInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlShader(shaderInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*shader = shaderInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
