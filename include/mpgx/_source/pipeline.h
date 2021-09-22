#pragma once
#include "mpgx/_source/shader.h"

#include <assert.h>

#define GL_NULL_UNIFORM_LOCATION -1

typedef struct _VkPipeline
{
	Window window;
	const char* name;
	Shader* shaders;
	uint8_t shaderCount;
	uint8_t drawMode;
	uint8_t polygonMode;
	uint8_t cullMode;
	uint8_t depthCompare;
	bool cullFace;
	bool clockwiseFrontFace;
	bool testDepth;
	bool writeDepth;
	bool clampDepth;
	bool restartPrimitive;
	bool discardRasterizer;
	float lineWidth;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
#if MPGX_SUPPORT_VULKAN
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
#endif
} _VkPipeline;
typedef struct _GlPipeline
{
	Window window;
	const char* name;
	Shader* shaders;
	uint8_t shaderCount;
	uint8_t drawMode;
	uint8_t polygonMode;
	uint8_t cullMode;
	uint8_t depthCompare;
	bool cullFace;
	bool clockwiseFrontFace;
	bool testDepth;
	bool writeDepth;
	bool clampDepth;
	bool restartPrimitive;
	bool discardRasterizer;
	float lineWidth;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	void* handle;
	GLuint glHandle;
	GLenum glDrawMode;
	GLenum _glPolygonMode;
	GLenum glCullMode;
	GLenum glDepthCompare;
} _GlPipeline;
union Pipeline
{
	_VkPipeline vk;
	_GlPipeline gl;
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkPipelineCreateInfo
{
	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo;
	VkPipelineLayoutCreateInfo layoutCreateInfo;
} VkPipelineCreateInfo;

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
inline static bool getVkPolygonMode(
	uint8_t polygonMode,
	VkPolygonMode* vkPolygonMode)
{
	if (polygonMode == POINT_POLYGON_MODE)
	{
		*vkPolygonMode = VK_POLYGON_MODE_POINT;
		return true;
	}
	else if (polygonMode == LINE_POLYGON_MODE)
	{
		*vkPolygonMode = VK_POLYGON_MODE_LINE;
		return true;
	}
	else if (polygonMode == FILL_POLYGON_MODE)
	{
		*vkPolygonMode = VK_POLYGON_MODE_FILL;
		return true;
	}
	else
	{
		return false;
	}
}
inline static bool getVkCullMode(
	uint8_t cullMode,
	bool cullFace,
	VkCullModeFlags* vkCullMode)
{
	if (cullFace == false)
	{
		*vkCullMode = VK_CULL_MODE_NONE;
		return true;
	}

	if (cullMode == FRONT_CULL_MODE)
	{
		*vkCullMode = VK_CULL_MODE_FRONT_BIT;
		return true;
	}
	else if (cullMode == BACK_CULL_MODE)
	{
		*vkCullMode = VK_CULL_MODE_BACK_BIT;
		return true;
	}
	else if (cullMode == FRONT_AND_BACK_CULL_MODE)
	{
		*vkCullMode = VK_CULL_MODE_FRONT_AND_BACK;
		return true;
	}
	else
	{
		return false;
	}
}

inline static Pipeline createVkPipeline(
	VkDevice device,
	VkRenderPass renderPass,
	VkPipelineCreateInfo* createInfo,
	Window window,
	const char* name,
	Shader* _shaders,
	uint8_t shaderCount,
	uint8_t drawMode,
	uint8_t polygonMode,
	uint8_t cullMode,
	uint8_t depthCompare,
	bool cullFace,
	bool clockwiseFrontFace,
	bool testDepth,
	bool writeDepth,
	bool clampDepth,
	bool restartPrimitive,
	bool discardRasterizer,
	float lineWidth,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle)
{
	Pipeline pipeline = malloc(
		sizeof(union Pipeline));

	if (pipeline == NULL)
		return NULL;

	Shader* shaders = malloc(
		shaderCount * sizeof(Shader));

	if (shaders == NULL)
	{
		free(pipeline);
		return NULL;
	}

	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (shaderStageCreateInfos == NULL)
	{
		free(shaders);
		free(pipeline);
		return NULL;
	}

	for (uint8_t i = 0; i < shaderCount; i++)
	{
		Shader shader = _shaders[i];
		assert(shader->vk.window == window);

		VkShaderStageFlagBits shaderStage;

		bool result = getVkShaderType(
			shader->vk.type,
			&shaderStage);

		if (result == false)
		{
			free(shaderStageCreateInfos);
			free(shaders);
			free(pipeline);
			return NULL;
		}

		VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
			VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
			NULL,
			0,
			shaderStage,
			shader->vk.handle,
			"main",
			NULL, // TODO: pass here shader dynamic constants
		};

		shaders[i] = shader;
		shaderStageCreateInfos[i] = shaderStageCreateInfo;
	}

	VkPrimitiveTopology vkPrimitiveTopology;
	VkPolygonMode vkPolygonMode;
	VkCullModeFlags vkCullMode;
	VkCompareOp vkDepthCompare;

	bool result = getVkDrawMode(
		drawMode,
		&vkPrimitiveTopology);
	result &= getVkPolygonMode(
		polygonMode,
		&vkPolygonMode);
	result &= getVkCullMode(
		cullMode,
		cullFace,
		&vkCullMode);
	result &= getVkCompareOperation(
		depthCompare,
		&vkDepthCompare);

	if (result == false)
	{
		free(shaderStageCreateInfos);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	VkFrontFace vkFrontFace = clockwiseFrontFace ?
		VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0,
		vkPrimitiveTopology,
		restartPrimitive ? VK_TRUE : VK_FALSE,
	};

	// TODO: tesselation stage

	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		NULL,
		0,
		1,
		NULL,
		1,
		NULL, // TODO: dynamic viewport/scissors
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT,
		NULL,
		0,
		clampDepth ? VK_TRUE : VK_FALSE,
		discardRasterizer ? VK_TRUE : VK_FALSE,
		vkPolygonMode,
		vkCullMode,
		vkFrontFace,
		VK_FALSE, // TODO: implement depth bias
		0.0f,
		0.0f,
		0.0f,
		lineWidth,
	};

	// TODO: multisampling
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_STREAM_CREATE_INFO_EXT,
		NULL,
		0,
		VK_SAMPLE_COUNT_1_BIT,
		VK_FALSE,
		0.0f,
		NULL,
		VK_FALSE,
		VK_FALSE,
	};

	VkPipelineDepthStencilStateCreateInfo depthStencilStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
		NULL,
		0,
		testDepth ? VK_TRUE : VK_FALSE,
		writeDepth ? VK_TRUE : VK_FALSE,
		vkDepthCompare,
		VK_FALSE, // TODO:
		VK_FALSE,
		{},
		{},
		0.0f,
		0.0f,
	};

	// TODO:
	VkPipelineColorBlendAttachmentState colorBlendAttachmentStateCreateInfo = {
		VK_FALSE,
		0,
		0,
		0,
		0,
		0,
		0,
		VK_COLOR_COMPONENT_R_BIT |
		VK_COLOR_COMPONENT_G_BIT |
		VK_COLOR_COMPONENT_B_BIT |
		VK_COLOR_COMPONENT_A_BIT,
	};

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		NULL,
		0,
		VK_FALSE, // TODO: logic operation
		0,
		1,
		&colorBlendAttachmentStateCreateInfo,
		{},
	};

	// TODO: do not make dynamic if not using custom scissors or viewport

	VkDynamicState dynamicStates[] = {
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR,
	};
	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		NULL,
		0,
		2,
		dynamicStates,
	};

	VkPipelineCacheCreateInfo cacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
	};

	VkPipelineCache cache;

	VkResult vkResult = vkCreatePipelineCache(
		device,
		&cacheCreateInfo,
		NULL,
		&cache);

	if (vkResult != VK_SUCCESS)
	{
		free(shaderStageCreateInfos);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	VkPipelineLayout layout;

	vkResult = vkCreatePipelineLayout(
		device,
		&createInfo->layoutCreateInfo,
		NULL,
		&layout);

	if (vkResult != VK_SUCCESS)
	{
		vkDestroyPipelineCache(
			device,
			cache,
			NULL);
		free(shaderStageCreateInfos);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		0,
		shaderCount,
		shaderStageCreateInfos,
		&createInfo->vertexInputStateCreateInfo,
		&inputAssemblyStateCreateInfo,
		NULL,
		&viewportStateCreateInfo,
		&rasterizationStateCreateInfo,
		&multisampleStateCreateInfo,
		&depthStencilStateCreateInfo,
		&colorBlendStateCreateInfo,
		&dynamicStateCreateInfo,
		layout,
		renderPass,
		0,
		NULL,
		-1
	};

	VkPipeline vkHandle;

	vkResult = vkCreateGraphicsPipelines(
		device,
		cache,
		1,
		&graphicsPipelineCreateInfo,
		NULL,
		&vkHandle);

	free(shaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
	{
		vkDestroyPipelineLayout(
			device,
			layout,
			NULL);
		vkDestroyPipelineCache(
			device,
			cache,
			NULL);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	pipeline->vk.window = window;
	pipeline->vk.name = name;
	pipeline->vk.shaders = shaders;
	pipeline->vk.shaderCount = shaderCount;
	pipeline->vk.drawMode = drawMode;
	pipeline->vk.polygonMode = polygonMode;
	pipeline->vk.cullMode = cullMode;
	pipeline->vk.depthCompare = depthCompare;
	pipeline->vk.cullFace = cullFace;
	pipeline->vk.clockwiseFrontFace = clockwiseFrontFace;
	pipeline->vk.testDepth = testDepth;
	pipeline->vk.writeDepth = writeDepth;
	pipeline->vk.clampDepth = clampDepth;
	pipeline->vk.restartPrimitive = restartPrimitive;
	pipeline->vk.discardRasterizer = discardRasterizer;
	pipeline->vk.lineWidth = lineWidth;
	pipeline->vk.onHandleDestroy = onHandleDestroy;
	pipeline->vk.onHandleBind = onHandleBind;
	pipeline->vk.onUniformsSet = onUniformsSet;
	pipeline->vk.handle = handle;
	pipeline->vk.cache = cache;
	pipeline->vk.layout = layout;
	pipeline->vk.vkHandle = vkHandle;
	return pipeline;
}
#endif

inline static bool getGlDrawMode(
	uint8_t drawMode,
	GLenum* glDrawMode)
{
	switch (drawMode)
	{
	default:
		return false;
	case POINT_LIST_DRAW_MODE:
		*glDrawMode = GL_POINTS;
		return true;
	case LINE_STRIP_DRAW_MODE:
		*glDrawMode = GL_LINE_STRIP;
		return true;
	case LINE_LOOP_DRAW_MODE:
		*glDrawMode = GL_LINE_LOOP;
		return true;
	case LINE_LIST_DRAW_MODE:
		*glDrawMode = GL_LINES;
		return true;
	case TRIANGLE_STRIP_DRAW_MODE:
		*glDrawMode = GL_TRIANGLE_STRIP;
		return true;
	case TRIANGLE_FAN_DRAW_MODE:
		*glDrawMode = GL_TRIANGLE_FAN;
		return true;
	case TRIANGLE_LIST_DRAW_MODE:
		*glDrawMode = GL_TRIANGLES;
		return true;
	}
}
inline static bool getGlPolygonMode(
	uint8_t polygonMode,
	GLenum* _glPolygonMode)
{
	if (polygonMode == POINT_POLYGON_MODE)
	{
		*_glPolygonMode = GL_POINT;
		return true;
	}
	else if (polygonMode == LINE_POLYGON_MODE)
	{
		*_glPolygonMode = GL_LINE;
		return true;
	}
	else if (polygonMode == FILL_POLYGON_MODE)
	{
		*_glPolygonMode = GL_FILL;
		return true;
	}
	else
	{
		return false;
	}
}
inline static bool getGlCullMode(
	uint8_t cullMode,
	GLenum* glCullMode)
{
	if (cullMode == FRONT_CULL_MODE)
	{
		*glCullMode = GL_FRONT;
		return true;
	}
	else if (cullMode == BACK_CULL_MODE)
	{
		*glCullMode = GL_BACK;
		return true;
	}
	else if (cullMode == FRONT_AND_BACK_CULL_MODE)
	{
		*glCullMode = GL_FRONT_AND_BACK;
		return true;
	}
	else
	{
		return false;
	}
}

inline static Pipeline createGlPipeline(
	Window window,
	const char* name,
	Shader* _shaders,
	uint8_t shaderCount,
	uint8_t drawMode,
	uint8_t polygonMode,
	uint8_t cullMode,
	uint8_t depthCompare,
	bool cullFace,
	bool clockwiseFrontFace,
	bool testDepth,
	bool writeDepth,
	bool clampDepth,
	float lineWidth,
	OnPipelineHandleDestroy onHandleDestroy,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	void* handle)
{
	Pipeline pipeline = malloc(
		sizeof(union Pipeline));

	if (pipeline == NULL)
		return NULL;

	Shader* shaders = malloc(
		shaderCount * sizeof(Shader));

	if (shaders == NULL)
	{
		free(pipeline);
		return NULL;
	}

	GLenum glDrawMode, _glPolygonMode,
		glCullMode, glDepthCompare;

	bool result = getGlDrawMode(
		drawMode,
		&glDrawMode);
	result &= getGlPolygonMode(
		polygonMode,
		&_glPolygonMode);

	if (cullFace == true)
	{
		result &= getGlCullMode(
			cullMode,
			&glCullMode);
	}
	else
	{
		glCullMode = GL_ZERO;
	}

	result &= getGlCompareOperation(
		depthCompare,
		&glDepthCompare);

	if (result == false)
	{
		free(shaders);
		free(pipeline);
		return NULL;
	}

	makeWindowContextCurrent(window);

	GLuint glHandle = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		assert(_shaders[i]->gl.window == window);
		Shader shader = shaders[i] = _shaders[i];

		glAttachShader(
			glHandle,
			shader->gl.handle);
	}

	glLinkProgram(glHandle);

	for (size_t i = 0; i < shaderCount; i++)
	{
		glDetachShader(
			glHandle,
			_shaders[i]->gl.handle);
	}

	GLint linkStatus;

	glGetProgramiv(
		glHandle,
		GL_LINK_STATUS,
		&linkStatus);

	assertOpenGL();

	if (linkStatus == GL_FALSE)
	{
		GLint length = 0;

		glGetProgramiv(
			glHandle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteProgram(glHandle);
				free(shaders);
				free(pipeline);
				return NULL;
			}

			glGetProgramInfoLog(
				glHandle,
				length,
				&length,
				(GLchar*)infoLog);

			fprintf(GL_INFO_LOG_OUT,
				"OpenGL program link error: %s\n",
				infoLog);

			free(infoLog);
		}

		assertOpenGL();

		glDeleteProgram(glHandle);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	pipeline->gl.window = window;
	pipeline->gl.name = name;
	pipeline->gl.shaders = shaders;
	pipeline->gl.shaderCount = shaderCount;
	pipeline->gl.drawMode = drawMode;
	pipeline->gl.polygonMode = polygonMode;
	pipeline->gl.cullMode = cullMode;
	pipeline->gl.depthCompare = depthCompare;
	pipeline->gl.cullFace = cullFace;
	pipeline->gl.clockwiseFrontFace = clockwiseFrontFace;
	pipeline->gl.testDepth = testDepth;
	pipeline->gl.writeDepth = writeDepth;
	pipeline->gl.clampDepth = clampDepth;
	pipeline->gl.restartPrimitive = false;
	pipeline->gl.discardRasterizer = false;
	pipeline->gl.lineWidth = lineWidth;
	pipeline->gl.onHandleDestroy = onHandleDestroy;
	pipeline->gl.onHandleBind = onHandleBind;
	pipeline->gl.onUniformsSet = onUniformsSet;
	pipeline->gl.handle = handle;
	pipeline->gl.glHandle = glHandle;
	pipeline->gl.glDrawMode = glDrawMode;
	pipeline->gl._glPolygonMode = _glPolygonMode;
	pipeline->gl.glCullMode = glCullMode;
	pipeline->gl.glDepthCompare = glDepthCompare;
	return pipeline;
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
	vkDestroyPipelineLayout(
		device,
		pipeline->vk.layout,
		NULL);
	vkDestroyPipelineCache(
		device,
		pipeline->vk.cache,
		NULL);

	free(pipeline->gl.shaders);
	free(pipeline);
}
#endif

inline static void destroyGlPipeline(
	Pipeline pipeline)
{
	makeWindowContextCurrent(
		pipeline->gl.window);

	glDeleteProgram(
		pipeline->gl.glHandle);
	assertOpenGL();

	free(pipeline->gl.shaders);
	free(pipeline);
}

#if MPGX_SUPPORT_VULKAN
inline static void bindVkPipeline(
	VkCommandBuffer commandBuffer,
	Pipeline pipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.vkHandle);
}
#endif

inline static void bindGlPipeline(
	Pipeline pipeline)
{
	// TODO: get viewport size from pipeline or dont set if custom enabled
	Vec2U size = getWindowFramebufferSize(
		pipeline->gl.window);

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	glPolygonMode(
		GL_FRONT_AND_BACK,
		pipeline->gl._glPolygonMode);

	if (pipeline->gl.cullFace == true)
	{
		glFrontFace(
			pipeline->gl.clockwiseFrontFace ?
			GL_CW : GL_CCW);
		glCullFace(pipeline->gl.glCullMode);
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	if (pipeline->gl.testDepth)
	{
		if (pipeline->gl.clampDepth)
		{
			glDepthRange(0.0f, 1.0f); // TODO:
			glEnable(GL_DEPTH_CLAMP);
		}
		else
		{
			glDisable(GL_DEPTH_CLAMP);
		}

		glDepthMask(
			pipeline->gl.writeDepth ?
			GL_TRUE : GL_FALSE);
		glDepthFunc(pipeline->gl.glDepthCompare);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	// TODO: color mask
	glColorMask(
		GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE);
	glPolygonOffset(0.0f, 0.0f);

	glUseProgram(pipeline->gl.glHandle);
	assertOpenGL();

	if (pipeline->gl.onHandleBind != NULL)
		pipeline->gl.onHandleBind(pipeline);
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
