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

#if MPGX_SUPPORT_OPENGL
#define OPENGL_SHADER_HEADER "#version 330 core\n"
#endif

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
	uint8_t _alignment[3];
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
	uint8_t _alignment[3];
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

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkShader(
	VkDevice device,
	Shader shader)
{
	assert(device);

	if (!shader)
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
	assert(device);
	assert(window);
	assert(type < SHADER_TYPE_COUNT);
	assert(code);
	assert(size > 0);
	assert(shader);

	Shader shaderInstance = calloc(1, sizeof(Shader_T));

	if (!shaderInstance)
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
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
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
		return vkToMpgxResult(vkResult);
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
	assert(glShaderType);

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
	if (!shader)
		return;

	makeGlWindowContextCurrent(
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
	assert(window);
	assert(type < SHADER_TYPE_COUNT);
	assert(code);
	assert(size > 0);
	assert(api < GRAPHICS_API_COUNT);
	assert(shader);

	Shader shaderInstance = calloc(1, sizeof(Shader_T));

	if (!shaderInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	shaderInstance->gl.window = window;
	shaderInstance->gl.type = type;

	GLenum glType;

	if (!getGlShaderType(type, &glType))
	{
		destroyGlShader(shaderInstance);
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	const char* sources[2] = {
		OPENGL_SHADER_HEADER,
		(const char*)code,
	};
	GLint lengths[2] = {
		(GLint)strlen(OPENGL_SHADER_HEADER),
		(GLint)size
	};

	makeGlWindowContextCurrent(window);

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

			if (!infoLog)
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

			fprintf(stderr,
				"OpenGL %s shader compile error:\n%s",
				typeString,
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		destroyGlShader(shaderInstance);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	GLenum glError = glGetError();

	if (glError != GL_NO_ERROR)
	{
		destroyGlShader(shaderInstance);
		return glToMpgxResult(glError);
	}

	*shader = shaderInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
