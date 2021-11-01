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
#include "mpgx/_source/shader.h"
#include "mpgx/_source/framebuffer.h"

typedef struct _BasePipeline
{
	Framebuffer framebuffer;
	Shader* shaders;
	size_t shaderCount;
	PipelineState state;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineHandleResize onHandleResize;
	OnPipelineHandleDestroy onHandleDestroy;
	void* handle;
#ifndef NDEBUG
	const char* name;
#endif
} _BasePipeline;
typedef struct _VkPipeline
{
	Framebuffer framebuffer;
	Shader* shaders;
	size_t shaderCount;
	PipelineState state;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineHandleResize onHandleResize;
	OnPipelineHandleDestroy onHandleDestroy;
	void* handle;
#ifndef NDEBUG
	const char* name;
#endif
#if MPGX_SUPPORT_VULKAN
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
#endif
} _VkPipeline;
typedef struct _GlPipeline
{
	Framebuffer framebuffer;
	Shader* shaders;
	size_t shaderCount;
	PipelineState state;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineHandleResize onHandleResize;
	OnPipelineHandleDestroy onHandleDestroy;
	void* handle;
#ifndef NDEBUG
	const char* name;
#endif
	GLuint glHandle;
	GLenum drawMode;
	GLenum polygonMode;
	GLenum cullMode;
	GLenum depthCompareOperator;
	GLenum srcColorBlendFactor;
	GLenum dstColorBlendFactor;
	GLenum srcAlphaBlendFactor;
	GLenum dstAlphaBlendFactor;
	GLenum colorBlendOperator;
	GLenum alphaBlendOperator;
	GLenum frontFace;
} _GlPipeline;
union Pipeline
{
	_BasePipeline base;
	_VkPipeline vk;
	_GlPipeline gl;
};

#if MPGX_SUPPORT_VULKAN
inline static bool getVkShaderType(
	ShaderType shaderType,
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
	case TESSELLATION_CONTROL_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_TESSELLATION_CONTROL_BIT;
		return true;
	case TESSELLATION_EVALUATION_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_TESSELLATION_EVALUATION_BIT;
		return true;
	case GEOMETRY_SHADER_TYPE:
		*vkShaderType = VK_SHADER_STAGE_GEOMETRY_BIT;
		return true;
	}
}
inline static bool getVkDrawMode(
	DrawMode drawMode,
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
	case LINE_LIST_WITH_ADJACENCY_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_LINE_LIST_WITH_ADJACENCY;
		return true;
	case LINE_STRIP_WITH_ADJACENCY_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_LINE_STRIP_WITH_ADJACENCY;
		return true;
	case TRIANGLE_LIST_WITH_ADJACENCY_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST_WITH_ADJACENCY;
		return true;
	case TRIANGLE_STRIP_WITH_ADJACENCY_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_STRIP_WITH_ADJACENCY;
		return true;
	case PATCH_LIST_DRAW_MODE:
		*vkDrawMode = VK_PRIMITIVE_TOPOLOGY_PATCH_LIST;
		return true;
	}
}
inline static bool getVkPolygonMode(
	PolygonMode polygonMode,
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
	CullMode cullMode,
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
inline static bool getVkBlendFactor(
	BlendFactor blendFactor,
	VkBlendFactor* vkBlendFactor)
{
	switch (blendFactor)
	{
	default:
		return false;
	case ZERO_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ZERO;
		return true;
	case ONE_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE;
		return true;
	case SRC_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		return true;
	case ONE_MINUS_SRC_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		return true;
	case DST_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		return true;
	case ONE_MINUS_DST_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		return true;
	case SRC_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		return true;
	case ONE_MINUS_SRC_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		return true;
	case DST_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		return true;
	case ONE_MINUS_DST_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_ALPHA;
		return true;
	case CONSTANT_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_CONSTANT_COLOR;
		return true;
	case ONE_MINUS_CONSTANT_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_COLOR;
		return true;
	case CONSTANT_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_CONSTANT_ALPHA;
		return true;
	case ONE_MINUS_CONSTANT_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_CONSTANT_ALPHA;
		return true;
	case SRC_ALPHA_SATURATE_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		return true;
	case SRC1_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
		return true;
	case ONE_MINUS_SRC1_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		return true;
	case SRC1_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;
		return true;
	case ONE_MINUS_SRC1_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		return true;
	}
}
inline static bool getVkBlendOperator(
	BlendOperator blendOperator,
	VkBlendOp* vkBlendOperator)
{
	switch (blendOperator)
	{
	default:
		return false;
	case ADD_BLEND_OPERATOR:
		*vkBlendOperator = VK_BLEND_OP_ADD;
		return true;
	case SUBTRACT_BLEND_OPERATOR:
		*vkBlendOperator = VK_BLEND_OP_SUBTRACT;
		return true;
	case REVERSE_SUBTRACT_BLEND_OPERATOR:
		*vkBlendOperator = VK_BLEND_OP_REVERSE_SUBTRACT;
		return true;
	case MIN_BLEND_OPERATOR:
		*vkBlendOperator = VK_BLEND_OP_MIN;
		return true;
	case MAX_BLEND_OPERATOR:
		*vkBlendOperator = VK_BLEND_OP_MAX;
		return true;
	}
}

inline static VkPipeline createVkPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	Window window,
	Shader* shaders,
	size_t shaderCount,
	PipelineState state,
	size_t colorAttachmentCount,
	VkPipelineCreateInfo* createInfo)
{
	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (shaderStageCreateInfos == NULL)
		return NULL;

	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader shader = shaders[i];
		assert(shader->vk.window == window);

		VkShaderStageFlagBits shaderStage;

		bool result = getVkShaderType(
			shader->vk.type,
			&shaderStage);

		if (result == false)
		{
			free(shaderStageCreateInfos);
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

		shaderStageCreateInfos[i] = shaderStageCreateInfo;
	}

	VkPrimitiveTopology primitiveTopology;
	VkPolygonMode polygonMode;
	VkCullModeFlags cullMode;
	VkCompareOp depthCompareOperator;

	VkBlendFactor
		srcColorBlendFactor, dstColorBlendFactor,
		srcAlphaBlendFactor, dstAlphaBlendFactor;
	VkBlendOp colorBlendOperator, alphaBlendOperator;

	bool result = getVkDrawMode(
		state.drawMode,
		&primitiveTopology);
	result &= getVkPolygonMode(
		state.polygonMode,
		&polygonMode);
	result &= getVkCullMode(
		state.cullMode,
		state.cullFace,
		&cullMode);
	result &= getVkCompareOperator(
		state.depthCompareOperator,
		&depthCompareOperator);
	result &= getVkBlendFactor(
		state.srcColorBlendFactor,
		&srcColorBlendFactor);
	result &= getVkBlendFactor(
		state.dstColorBlendFactor,
		&dstColorBlendFactor);
	result &= getVkBlendFactor(
		state.srcAlphaBlendFactor,
		&srcAlphaBlendFactor);
	result &= getVkBlendFactor(
		state.dstAlphaBlendFactor,
		&dstAlphaBlendFactor);
	result &= getVkBlendOperator(
		state.colorBlendOperator,
		&colorBlendOperator);
	result &= getVkBlendOperator(
		state.alphaBlendOperator,
		&alphaBlendOperator);

	if (result == false)
	{
		free(shaderStageCreateInfos);
		return NULL;
	}

	VkFrontFace vkFrontFace = state.clockwiseFrontFace ?
		VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		NULL,
		0,
		createInfo->vertexBindingDescriptionCount,
		createInfo->vertexBindingDescriptions,
		createInfo->vertexAttributeDescriptionCount,
		createInfo->vertexAttributeDescriptions,
	};

	VkPipelineInputAssemblyStateCreateInfo inputAssemblyStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
		NULL,
		0,
		primitiveTopology,
		state.restartPrimitive ? VK_TRUE : VK_FALSE,
	};

	// TODO: tesselation stage

	VkDynamicState dynamicStates[2];
	uint32_t dynamicStateCount = 0;

	bool dynamicViewport =
		state.viewport.z +
		state.viewport.w == 0;

	if (dynamicViewport == true)
	{
		dynamicStates[dynamicStateCount++] =
			VK_DYNAMIC_STATE_VIEWPORT;
	}

	bool dynamicScissor =
		state.scissor.z +
		state.scissor.w == 0;

	if (dynamicScissor == true)
	{
		dynamicStates[dynamicStateCount++] =
			VK_DYNAMIC_STATE_SCISSOR;
	}

	VkViewport viewport = {
		(float)state.viewport.x,
		(float)state.viewport.y,
		(float)state.viewport.z,
		(float)state.viewport.w,
		state.depthRange.x,
		state.depthRange.y,
	};
	VkRect2D scissor = {
		{
			(int32_t)state.scissor.x,
			(int32_t)state.scissor.y,
		},
		{
			(uint32_t)state.scissor.z,
			(uint32_t)state.scissor.w,
		},
	};
	VkPipelineViewportStateCreateInfo viewportStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
		NULL,
		0,
		1,
		dynamicViewport ? NULL : &viewport,
		1,
		dynamicScissor ? NULL : &scissor,
	};

	VkPipelineRasterizationStateCreateInfo rasterizationStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
		NULL,
		0,
		state.clampDepth ? VK_TRUE : VK_FALSE,
		state.discardRasterizer ? VK_TRUE : VK_FALSE,
		polygonMode,
		cullMode,
		vkFrontFace,
		state.enableDepthBias ? VK_TRUE : VK_FALSE,
		state.depthBias.x,
		0.0f, // TODO:
		state.depthBias.y,
		state.lineWidth,
	};

	// TODO: multisampling
	VkPipelineMultisampleStateCreateInfo multisampleStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
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
		state.testDepth ? VK_TRUE : VK_FALSE,
		state.writeDepth ? VK_TRUE : VK_FALSE,
		depthCompareOperator,
		VK_FALSE, // TODO:
		VK_FALSE,
		{
			0,
			0,
			0,
			0,
			0,
			0,
			0
		},
		{
			0,
			0,
			0,
			0,
			0,
			0,
			0
		},
		0.0f,
		0.0f,
	};

	VkColorComponentFlags vkColorWriteMask = 0;

	if (state.colorComponentWriteMask & RED_COLOR_COMPONENT)
		vkColorWriteMask |= VK_COLOR_COMPONENT_R_BIT;
	if (state.colorComponentWriteMask & GREEN_COLOR_COMPONENT)
		vkColorWriteMask |= VK_COLOR_COMPONENT_G_BIT;
	if (state.colorComponentWriteMask & BLUE_COLOR_COMPONENT)
		vkColorWriteMask |= VK_COLOR_COMPONENT_B_BIT;
	if (state.colorComponentWriteMask & ALPHA_COLOR_COMPONENT)
		vkColorWriteMask |= VK_COLOR_COMPONENT_A_BIT;

	// TODO: add ability to set separated pipeline color blend values
	// possibly when OpenGL support will be removed

	VkPipelineColorBlendAttachmentState* colorBlendAttachmentStates = malloc(
		colorAttachmentCount * sizeof(VkPipelineColorBlendAttachmentState));

	if (colorBlendAttachmentStates == NULL)
	{
		free(shaderStageCreateInfos);
		return NULL;
	}

	VkPipelineColorBlendAttachmentState colorBlendAttachmentState = {
		state.enableBlend ? VK_TRUE : VK_FALSE,
		srcColorBlendFactor,
		dstColorBlendFactor,
		colorBlendOperator,
		srcAlphaBlendFactor,
		dstAlphaBlendFactor,
		alphaBlendOperator,
		vkColorWriteMask,
	};

	for (size_t i = 0; i < colorAttachmentCount; i++)
		colorBlendAttachmentStates[i] = colorBlendAttachmentState;

	VkPipelineColorBlendStateCreateInfo colorBlendStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
		NULL,
		0,
		VK_FALSE, // TODO: logic operation
		0,
		(uint32_t)colorAttachmentCount,
		colorBlendAttachmentStates,
		{
			state.blendColor.x,
			state.blendColor.y,
			state.blendColor.z,
			state.blendColor.w,
		},
	};

	VkPipelineDynamicStateCreateInfo dynamicStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
		NULL,
		0,
		dynamicStateCount,
		dynamicStates,
	};

	VkGraphicsPipelineCreateInfo graphicsPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
		NULL,
		0,
		(uint32_t)shaderCount,
		shaderStageCreateInfos,
		&vertexInputStateCreateInfo,
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

	VkPipeline handle;

	VkResult vkResult = vkCreateGraphicsPipelines(
		device,
		cache,
		1,
		&graphicsPipelineCreateInfo,
		NULL,
		&handle);

	free(colorBlendAttachmentStates);
	free(shaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
		return NULL;

	return handle;
}
inline static bool recreateVkPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	Pipeline pipeline,
	size_t colorAttachmentCount,
	VkPipelineCreateInfo* createInfo)
{
	VkPipeline handle = createVkPipelineHandle(
		device,
		renderPass,
		pipeline->vk.cache,
		pipeline->vk.layout,
		pipeline->vk.framebuffer->vk.window,
		pipeline->vk.shaders,
		pipeline->vk.shaderCount,
		pipeline->vk.state,
		colorAttachmentCount,
		createInfo);

	if (handle == NULL)
		return false;

	vkDestroyPipeline(
		device,
		pipeline->vk.vkHandle,
		NULL);

	pipeline->vk.vkHandle = handle;
	return true;
}

inline static Pipeline createVkPipeline(
	VkDevice device,
	VkPipelineCreateInfo* createInfo,
	Framebuffer framebuffer,
	const char* name,
	Shader* _shaders,
	size_t shaderCount,
	PipelineState state,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineHandleResize onHandleResize,
	OnPipelineHandleDestroy onHandleDestroy,
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

	for (size_t i = 0; i < shaderCount; i++)
		shaders[i] = _shaders[i];

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
		free(shaders);
		free(pipeline);
		return NULL;
	}

	VkPipelineLayoutCreateInfo layoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		createInfo->setLayoutCount,
		createInfo->setLayouts,
		createInfo->pushConstantRangeCount,
		createInfo->pushConstantRanges,
	};

	VkPipelineLayout layout;

	vkResult = vkCreatePipelineLayout(
		device,
		&layoutCreateInfo,
		NULL,
		&layout);

	if (vkResult != VK_SUCCESS)
	{
		vkDestroyPipelineCache(
			device,
			cache,
			NULL);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	VkPipeline vkHandle = createVkPipelineHandle(
		device,
		framebuffer->vk.renderPass,
		cache,
		layout,
		framebuffer->vk.window,
		shaders,
		shaderCount,
		state,
		framebuffer->vk.colorAttachmentCount,
		createInfo);

	if (vkHandle == NULL)
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

	pipeline->vk.framebuffer = framebuffer;
	pipeline->vk.shaders = shaders;
	pipeline->vk.shaderCount = shaderCount;
	pipeline->vk.state = state;

	pipeline->vk.onHandleBind = onHandleBind;
	pipeline->vk.onUniformsSet = onUniformsSet;
	pipeline->vk.onHandleResize = onHandleResize;
	pipeline->vk.onHandleDestroy = onHandleDestroy;
	pipeline->vk.handle = handle;
#ifndef NDEBUG
	pipeline->vk.name = name;
#endif
	pipeline->vk.cache = cache;
	pipeline->vk.layout = layout;
	pipeline->vk.vkHandle = vkHandle;
	return pipeline;
}
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
inline static void bindVkPipeline(
	VkCommandBuffer commandBuffer,
	Pipeline pipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.vkHandle);

	if (pipeline->vk.onHandleBind != NULL)
		pipeline->vk.onHandleBind(pipeline);
}
#endif

inline static bool getGlDrawMode(
	DrawMode drawMode,
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
	PolygonMode polygonMode,
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
	CullMode cullMode,
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
inline static bool getGlBlendFactor(
	BlendFactor blendFactor,
	GLenum* glBlendFactor)
{
	switch (blendFactor)
	{
	default:
		return false;
	case ZERO_BLEND_FACTOR:
		*glBlendFactor = GL_ZERO;
		return true;
	case ONE_BLEND_FACTOR:
		*glBlendFactor = GL_ONE;
		return true;
	case SRC_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_COLOR;
		return true;
	case ONE_MINUS_SRC_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_SRC_COLOR;
		return true;
	case DST_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_DST_COLOR;
		return true;
	case ONE_MINUS_DST_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_DST_COLOR;
		return true;
	case SRC_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_ALPHA;
		return true;
	case ONE_MINUS_SRC_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
		return true;
	case DST_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_DST_ALPHA;
		return true;
	case ONE_MINUS_DST_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_DST_ALPHA;
		return true;
	case CONSTANT_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_CONSTANT_COLOR;
		return true;
	case ONE_MINUS_CONSTANT_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_CONSTANT_COLOR;
		return true;
	case CONSTANT_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_CONSTANT_ALPHA;
		return true;
	case ONE_MINUS_CONSTANT_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_CONSTANT_ALPHA;
		return true;
	case SRC_ALPHA_SATURATE_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_ALPHA_SATURATE;
		return true;
	}
}
inline static bool getGlBlendOperator(
	BlendOperator blendOperator,
	GLenum* glBlendOperator)
{
	switch (blendOperator)
	{
	default:
		return false;
	case ADD_BLEND_OPERATOR:
		*glBlendOperator = GL_FUNC_ADD;
		return true;
	case SUBTRACT_BLEND_OPERATOR:
		*glBlendOperator = GL_FUNC_SUBTRACT;
		return true;
	case REVERSE_SUBTRACT_BLEND_OPERATOR:
		*glBlendOperator = GL_FUNC_REVERSE_SUBTRACT;
		return true;
	case MIN_BLEND_OPERATOR:
		*glBlendOperator = GL_MIN;
		return true;
	case MAX_BLEND_OPERATOR:
		*glBlendOperator = GL_MAX;
		return true;
	}
}
inline static Pipeline createGlPipeline(
	Framebuffer framebuffer,
	const char* name,
	Shader* _shaders,
	size_t shaderCount,
	PipelineState state,
	OnPipelineHandleBind onHandleBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineHandleResize onHandleResize,
	OnPipelineHandleDestroy onHandleDestroy,
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

	GLenum drawMode, polygonMode,
		cullMode, depthCompareOperator,
		srcColorBlendFactor, dstColorBlendFactor,
		srcAlphaBlendFactor, dstAlphaBlendFactor,
		colorBlendOperator, alphaBlendOperator;

	bool result = getGlDrawMode(
		state.drawMode,
		&drawMode);
	result &= getGlPolygonMode(
		state.polygonMode,
		&polygonMode);

	if (state.cullFace == true)
	{
		result &= getGlCullMode(
			state.cullMode,
			&cullMode);
	}
	else
	{
		cullMode = GL_ZERO;
	}

	result &= getGlCompareOperator(
		state.depthCompareOperator,
		&depthCompareOperator);
	result &= getGlBlendFactor(
		state.srcColorBlendFactor,
		&srcColorBlendFactor);
	result &= getGlBlendFactor(
		state.dstColorBlendFactor,
		&dstColorBlendFactor);
	result &= getGlBlendFactor(
		state.srcAlphaBlendFactor,
		&srcAlphaBlendFactor);
	result &= getGlBlendFactor(
		state.dstAlphaBlendFactor,
		&dstAlphaBlendFactor);
	result &= getGlBlendOperator(
		state.colorBlendOperator,
		&colorBlendOperator);
	result &= getGlBlendOperator(
		state.colorBlendOperator,
		&alphaBlendOperator);

	if (result == false)
	{
		free(shaders);
		free(pipeline);
		return NULL;
	}

	GLenum frontFace =
		state.clockwiseFrontFace == true ?
		GL_CW : GL_CCW;

	Window window = framebuffer->gl.window;
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

			printf("OpenGL program link error:\n%s",
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		glDeleteProgram(glHandle);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		glDeleteProgram(glHandle);
		free(shaders);
		free(pipeline);
		return NULL;
	}

	pipeline->gl.framebuffer = framebuffer;
	pipeline->gl.shaders = shaders;
	pipeline->gl.shaderCount = shaderCount;
	pipeline->gl.state = state;
	pipeline->gl.onHandleBind = onHandleBind;
	pipeline->gl.onUniformsSet = onUniformsSet;
	pipeline->gl.onHandleResize = onHandleResize;
	pipeline->gl.onHandleDestroy = onHandleDestroy;
	pipeline->gl.handle = handle;
#ifndef NDEBUG
	pipeline->gl.name = name;
#endif
	pipeline->gl.glHandle = glHandle;
	pipeline->gl.drawMode = drawMode;
	pipeline->gl.polygonMode = polygonMode;
	pipeline->gl.cullMode = cullMode;
	pipeline->gl.depthCompareOperator = depthCompareOperator;
	pipeline->gl.srcColorBlendFactor = srcColorBlendFactor;
	pipeline->gl.dstColorBlendFactor = dstColorBlendFactor;
	pipeline->gl.srcAlphaBlendFactor = srcAlphaBlendFactor;
	pipeline->gl.dstAlphaBlendFactor = dstAlphaBlendFactor;
	pipeline->gl.colorBlendOperator = colorBlendOperator;
	pipeline->gl.alphaBlendOperator = alphaBlendOperator;
	pipeline->gl.frontFace = frontFace;
	return pipeline;
}
inline static void destroyGlPipeline(
	Pipeline pipeline)
{
	makeWindowContextCurrent(
		pipeline->gl.framebuffer->gl.window);

	glDeleteProgram(
		pipeline->gl.glHandle);
	assertOpenGL();

	free(pipeline->gl.shaders);
	free(pipeline);
}
inline static void bindGlPipeline(
	Pipeline pipeline)
{
	Vec4I viewport = pipeline->gl.state.viewport;

	if (viewport.z + viewport.w > 0)
	{
		Vec2F depthRange = pipeline->gl.state.depthRange;
		glViewport(
			(GLint)viewport.x,
			(GLint)viewport.y,
			(GLsizei)viewport.z,
			(GLsizei)viewport.w);
		glDepthRange(
			depthRange.x,
			depthRange.y);
	}

	Vec4I scissor = pipeline->gl.state.scissor;

	if (scissor.z + scissor.w > 0)
	{
		glScissor(
			(GLint)scissor.x,
			(GLint)scissor.y,
			(GLsizei)scissor.z,
			(GLsizei)scissor.w);
		glEnable(GL_SCISSOR_TEST);
	}
	else
	{
		glDisable(GL_SCISSOR_TEST);
	}

	glPolygonMode(
		GL_FRONT_AND_BACK,
		pipeline->gl.polygonMode);

	if (pipeline->gl.state.cullFace == true)
	{
		glFrontFace(pipeline->gl.frontFace);
		glCullFace(pipeline->gl.cullMode);
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	ColorComponent colorMask =
		pipeline->gl.state.colorComponentWriteMask;

	glColorMask(
		colorMask & RED_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & GREEN_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & BLUE_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & ALPHA_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE);

	if (pipeline->gl.state.testDepth == true)
	{
		if (pipeline->gl.state.clampDepth == true)
			glEnable(GL_DEPTH_CLAMP);
		else
			glDisable(GL_DEPTH_CLAMP);

		glDepthMask(
			pipeline->gl.state.writeDepth == true ?
			GL_TRUE : GL_FALSE);
		glDepthFunc(pipeline->gl.depthCompareOperator);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (pipeline->gl.state.enableDepthBias == true)
	{
		Vec2F depthBias = pipeline->gl.state.depthBias;
		glPolygonOffset(
			depthBias.y,
			depthBias.x);
		glEnable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (pipeline->gl.state.enableBlend == true)
	{
		Vec4F blendColor =
			pipeline->gl.state.blendColor;

		glBlendFuncSeparate(
			pipeline->gl.srcColorBlendFactor,
			pipeline->gl.dstColorBlendFactor,
			pipeline->gl.srcAlphaBlendFactor,
			pipeline->gl.dstAlphaBlendFactor);
		glBlendEquationSeparate(
			pipeline->gl.colorBlendOperator,
			pipeline->gl.alphaBlendOperator);
		glBlendColor(
			blendColor.x,
			blendColor.y,
			blendColor.z,
			blendColor.w);
		glEnable(GL_BLEND);
	}
	else
	{
		glDisable(GL_BLEND);
	}

	if (pipeline->gl.state.restartPrimitive == true)
		glEnable(GL_PRIMITIVE_RESTART);
	else
		glDisable(GL_PRIMITIVE_RESTART);

	// TODO:
	glDisable(GL_STENCIL_TEST);

	glUseProgram(pipeline->gl.glHandle);
	assertOpenGL();

	if (pipeline->gl.onHandleBind != NULL)
		pipeline->gl.onHandleBind(pipeline);
}
inline static bool getGlUniformLocation(
	GLuint program,
	const GLchar* name,
	GLint* _location)
{
	GLint location = glGetUniformLocation(
		program,
		name);

	if (location == -1)
	{
		printf("Failed to get '%s' uniform location.\n",
			name);
		return false;
	}

	*_location = location;
	return true;
}
inline static GLuint getGlUniformBlockIndex(
	GLuint program,
	const GLchar* name,
	GLuint* _blockIndex)
{
	GLuint blockIndex = glGetUniformBlockIndex(
		program,
		name);

	if (blockIndex == GL_INVALID_INDEX)
	{
		printf("Failed to get '%s' uniform block index.\n",
			name);
		return false;
	}

	*_blockIndex = blockIndex;
	return true;
}
