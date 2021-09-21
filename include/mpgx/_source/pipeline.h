#pragma once
#include "mpgx/_source/opengl.h"
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/shader.h"

#include <assert.h>

#define GL_NULL_UNIFORM_LOCATION -1

typedef struct _VkPipeline
{
	Window window;
	const char* name;
	uint8_t drawMode;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
#if MPGX_SUPPORT_VULKAN
	VkPipelineCache cache;
	VkPipeline vkHandle;
#endif
} _VkPipeline;
typedef struct _GlPipeline
{
	Window window;
	const char* name;
	uint8_t drawMode;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
} _GlPipeline;
union Pipeline
{
	_VkPipeline vk;
	_GlPipeline gl;
};

#if MPGX_SUPPORT_VULKAN
inline static bool getVkShaderType(
	uint8_t shaderType,
	VkShaderStageFlagBits* vkShaderType)
{
	switch (shaderType)
	{
	default:
		return false;
	case VERTEX_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_VERTEX_BIT;
		return true;
	case FRAGMENT_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_FRAGMENT_BIT;
		return true;
	case COMPUTE_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_COMPUTE_BIT;
		return true;
	}
}
inline static bool getVkDrawMode(
	uint8_t drawMode,
	VkPrimitiveTopology* vkDrawMode)
{
	switch (drawMode)
	{
	default:
		return false;
	case POINT_LIST_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_POINT_LIST;
		return true;
	case LINE_STRIP_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP;
		return true;
	case LINE_LIST_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_LINE_LIST;
		return true;
	case TRIANGLE_STRIP_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP;
		return true;
	case TRIANGLE_FAN_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN;
		return true;
	case TRIANGLE_LIST_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
		return true;
	}
}

inline static Pipeline createVkPipeline(
	VkDevice device,
	Window window,
	const char* name,
	uint8_t drawMode,
	bool restartPrimitive,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle,
	Shader* shaders,
	uint8_t shaderCount,
	VkPipelineVertexInputStateCreateInfo* pipelineVertexInputStateCreateInfo)
{
	Pipeline pipeline = malloc(
		sizeof(union Pipeline));

	if (pipeline == NULL)
		return NULL;

	VkPipelineShaderStageCreateInfo* pipelineShaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (pipelineShaderStageCreateInfos == NULL)
	{
		free(pipeline);
		return NULL;
	}

	for (uint8_t i = 0; i < shaderCount; i++)
	{
		Shader shader = shaders[i];

		VkShaderStageFlagBits shaderStage;

		bool result = getVkShaderType(
			shader->vk.type,
			&shaderStage);

		if (result == false)
		{
			free(pipelineShaderStageCreateInfos);
			free(pipeline);
			return NULL;
		}

		VkPipelineShaderStageCreateInfo pipelineShaderStageCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			NULL,
			0,
			shaderStage,
			shader->vk.handle,
			"main",
			NULL, // TODO: pass here shader dynamic constants
		};

		pipelineShaderStageCreateInfos[i] = pipelineShaderStageCreateInfo;
	}

	VkPrimitiveTopology primitiveTopology;

	bool result = getVkDrawMode(
		drawMode,
		&primitiveTopology);

	if (result == false)
	{
		free(pipelineShaderStageCreateInfos);
		free(pipeline);
		return NULL;
	}

	VkPipelineInputAssemblyStateCreateInfo pipelineInputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0,
		primitiveTopology,
		restartPrimitive ? VK_TRUE : VK_FALSE,
	};

	// TODO: tesselation stage

	VkPipelineViewportStateCreateInfo pipelineViewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		NULL,
		0,
		1,
		NULL,
		1,
		NULL, // TODO: dynamic viewport/scissors
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		0,
		shaderCount,
		pipelineShaderStageCreateInfos,
		pipelineVertexInputStateCreateInfo,
		&pipelineInputAssemblyStateCreateInfo,
		NULL,

	};

	VkPipelineCacheCreateInfo pipelineCacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
	};

	VkPipelineCache cache;

	VkResult vkResult = vkCreatePipelineCache(
		device,
		&pipelineCacheCreateInfo,
		NULL,
		&cache);

	if (vkResult != VK_SUCCESS)
	{
		free(pipelineShaderStageCreateInfos);
		free(pipeline);
		return NULL;
	}

	VkPipeline vkHandle;

	vkResult = vkCreateGraphicsPipelines(
		device,
		cache,
		1,
		&graphicsPipelineCreateInfo,
		NULL,
		&vkHandle);

	free(pipelineShaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
	{
		// TODO:

		vkDestroyPipelineCache(
			device,
			cache,
			NULL);
		free(pipeline);
		return NULL;
	}

	pipeline->vk.window = window;
	pipeline->vk.name = name;
	pipeline->vk.drawMode = drawMode;
	pipeline->vk.onHandleDestroy = onHandleDestroy;
	pipeline->vk.onHandleBind = onHandleBind;
	pipeline->vk.onUniformsSet = onUniformsSet;
	pipeline->vk.handle = handle;
	pipeline->vk.cache = cache;
	pipeline->vk.vkHandle = vkHandle;
	return pipeline;
}
#endif

inline static GLuint createGlPipeline(
	Window window,
	Shader* shaders,
	uint8_t shaderCount)
{
	assert(window != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);

	makeWindowContextCurrent(window);

	GLuint program = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		assert(getShaderWindow(shaders[i]) == window);

		GLuint handle = (GLuint)(uintptr_t)
			getShaderHandle(shaders[i]);

		glAttachShader(
			program,
			handle);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		GLuint handle = (GLuint)(uintptr_t)
			getShaderHandle(shaders[i]);

		glDetachShader(
			program,
			handle);
	}

	GLint result;

	glGetProgramiv(
		program,
		GL_LINK_STATUS,
		&result);

	if (result == GL_FALSE)
	{
		GLint length = 0;

		glGetProgramiv(
			program,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteProgram(program);
				return GL_ZERO;
			}

			glGetProgramInfoLog(
				program,
				length,
				&length,
				(GLchar*)infoLog);

			fprintf(GL_INFO_LOG_OUT,
				"OpenGL program link error: %s\n",
				infoLog);

			free(infoLog);
		}

		assertOpenGL();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	assertOpenGL();
	return program;
}

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkPipeline(
	VkDevice device,
	Pipeline pipeline)
{
	vkDestroyPipeline(
		device,
		pipeline->vk.vkHandle,
		NULL);
	vkDestroyPipelineCache(
		device,
		pipeline->vk.cache,
		NULL);
	pipeline->vk.onHandleDestroy(
		pipeline->vk.window,
		pipeline->vk.handle);
	free(pipeline);
}
#endif

inline static void destroyGlPipeline(
	Window window,
	GLuint pipeline)
{
	assert(window != NULL);
	makeWindowContextCurrent(window);
	glDeleteProgram(pipeline);
	assertOpenGL();
}

inline static GLint getGlUniformLocation(
	GLuint program,
	const GLchar* name)
{
	GLint uniformLocation = glGetUniformLocation(
		program,
		name);

#ifndef NDEBUG
	if (uniformLocation == GL_NULL_UNIFORM_LOCATION)
	{
		fprintf(stderr,
			"Failed to get '%s' uniform location.\n",
			name);
	}
#endif

	return uniformLocation;
}
inline static GLuint getGlUniformBlockIndex(
	GLuint program,
	const GLchar* name)
{
	GLuint uniformBlockIndex = glGetUniformBlockIndex(
		program,
		name);

#ifndef NDEBUG
	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		fprintf(stderr,
			"Failed to get '%s' uniform block index.\n",
			name);
	}
#endif

	return uniformBlockIndex;
}
