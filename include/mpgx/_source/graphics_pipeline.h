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
#include "mpgx/_source/shader.h"
#include "mpgx/_source/framebuffer.h"

typedef struct BaseGraphicsPipeline_T
{
	Framebuffer framebuffer;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
	GraphicsPipelineState state;
#ifndef NDEBUG
	const char* name;
#endif
} BaseGraphicsPipeline_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkGraphicsPipeline_T
{
	Framebuffer framebuffer;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
	GraphicsPipelineState state;
#ifndef NDEBUG
	const char* name;
#endif
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
} VkGraphicsPipeline_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlGraphicsPipeline_T
{
	Framebuffer framebuffer;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
	GraphicsPipelineState state;
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
} GlGraphicsPipeline_T;
#endif
union GraphicsPipeline_T
{
	BaseGraphicsPipeline_T base;
#if MPGX_SUPPORT_VULKAN
	VkGraphicsPipeline_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlGraphicsPipeline_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
inline static bool getVkDrawMode(
	DrawMode drawMode,
	VkPrimitiveTopology* vkDrawMode)
{
	assert(drawMode < DRAW_MODE_COUNT);
	assert(vkDrawMode);

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
	assert(polygonMode < POLYGON_MODE_COUNT);
	assert(vkPolygonMode);

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
	assert(cullMode < CULL_MODE_COUNT);
	assert(vkCullMode);

	if (!cullFace)
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
	assert(blendFactor < BLEND_FACTOR_COUNT);
	assert(vkBlendFactor);

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
	case SOURCE_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_COLOR;
		return true;
	case ONE_MINUS_SOURCE_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_COLOR;
		return true;
	case DESTINATION_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_DST_COLOR;
		return true;
	case ONE_MINUS_DESTINATION_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_DST_COLOR;
		return true;
	case SOURCE_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
		return true;
	case ONE_MINUS_SOURCE_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
		return true;
	case DESTINATION_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_DST_ALPHA;
		return true;
	case ONE_MINUS_DESTINATION_ALPHA_BLEND_FACTOR:
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
	case SOURCE_ALPHA_SATURATE_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA_SATURATE;
		return true;
	case SOURCE_ONE_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC1_COLOR;
		return true;
	case ONE_MINUS_SOURCE_ONE_COLOR_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_COLOR;
		return true;
	case SOURCE_ONE_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_SRC1_ALPHA;
		return true;
	case ONE_MINUS_SOURCE_ONE_ALPHA_BLEND_FACTOR:
		*vkBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC1_ALPHA;
		return true;
	}
}
inline static bool getVkBlendOperator(
	BlendOperator blendOperator,
	VkBlendOp* vkBlendOperator)
{
	assert(blendOperator < BLEND_OPERATOR_COUNT);
	assert(vkBlendOperator);

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

inline static MpgxResult createVkGraphicsPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipelineState state,
	size_t colorAttachmentCount,
	const VkGraphicsPipelineCreateData* createData,
	VkPipeline* handle)
{
	assert(device);
	assert(renderPass);
	assert(cache);
	assert(layout);
	assert(shaders);
	assert(shaderCount > 0);
	assert(createData);
	assert(handle);

	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (!shaderStageCreateInfos)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
		"main",
		NULL, // TODO: pass here shader dynamic constants
	};

	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader shader = shaders[i];
		shaderStageCreateInfo.stage = shader->vk.stage;
		shaderStageCreateInfo.module = shader->vk.handle;
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

	if (!result)
	{
		free(shaderStageCreateInfos);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkFrontFace vkFrontFace = state.clockwiseFrontFace ?
		VK_FRONT_FACE_CLOCKWISE : VK_FRONT_FACE_COUNTER_CLOCKWISE;

	VkPipelineVertexInputStateCreateInfo vertexInputStateCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
		NULL,
		0,
		createData->vertexBindingDescriptionCount,
		createData->vertexBindingDescriptions,
		createData->vertexAttributeDescriptionCount,
		createData->vertexAttributeDescriptions,
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

	bool dynamicViewport = state.viewport.z + state.viewport.w == 0;

	if (dynamicViewport)
		dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_VIEWPORT;

	bool dynamicScissor = state.scissor.z + state.scissor.w == 0;

	if (dynamicScissor)
		dynamicStates[dynamicStateCount++] = VK_DYNAMIC_STATE_SCISSOR;

	// TODO: fix different Vulkan viewport/scissor
	// coordinate system from OpenGL

	VkViewport viewport = {
		(float)state.viewport.x,
		(float)state.viewport.y,
		(float)state.viewport.z,
		(float)state.viewport.w,
		state.depthRange.x,
		state.depthRange.y,
	};
	VkRect2D scissor = {
		(int32_t)state.scissor.x,
		(int32_t)state.scissor.y,
		(uint32_t)state.scissor.z,
		(uint32_t)state.scissor.w,
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

	VkPipelineColorBlendAttachmentState* colorBlendAttachmentStates;

	if (colorAttachmentCount > 0)
	{
		colorBlendAttachmentStates = malloc(
			colorAttachmentCount * sizeof(VkPipelineColorBlendAttachmentState));

		if (!colorBlendAttachmentStates)
		{
			free(shaderStageCreateInfos);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}
	}
	else
	{
		colorBlendAttachmentStates = NULL;
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
		0,
	};

	VkPipeline handleInstance;

	VkResult vkResult = vkCreateGraphicsPipelines(
		device,
		cache,
		1,
		&graphicsPipelineCreateInfo,
		NULL,
		&handleInstance);

	free(colorBlendAttachmentStates);
	free(shaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*handle = handleInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult recreateVkGraphicsPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	GraphicsPipeline graphicsPipeline,
	size_t colorAttachmentCount,
	const VkGraphicsPipelineCreateData* createData)
{
	assert(device);
	assert(renderPass);
	assert(graphicsPipeline);
	assert(createData);

	VkPipeline handle;

	MpgxResult mpgxResult = createVkGraphicsPipelineHandle(
		device,
		renderPass,
		graphicsPipeline->vk.cache,
		graphicsPipeline->vk.layout,
		graphicsPipeline->vk.shaders,
		graphicsPipeline->vk.shaderCount,
		graphicsPipeline->vk.state,
		colorAttachmentCount,
		createData,
		&handle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	vkDestroyPipeline(
		device,
		graphicsPipeline->vk.vkHandle,
		NULL);

	graphicsPipeline->vk.vkHandle = handle;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkGraphicsPipeline(
	VkDevice device,
	GraphicsPipeline graphicsPipeline,
	bool destroyShaders)
{
	assert(device);

	if (!graphicsPipeline)
		return;

	vkDestroyPipeline(
		device,
		graphicsPipeline->vk.vkHandle,
		NULL);
	vkDestroyPipelineLayout(
		device,
		graphicsPipeline->vk.layout,
		NULL);
	vkDestroyPipelineCache(
		device,
		graphicsPipeline->vk.cache,
		NULL);

	if (destroyShaders)
	{
		Shader* shaders = graphicsPipeline->vk.shaders;
		size_t shaderCount = graphicsPipeline->vk.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(graphicsPipeline->vk.shaders);
	free(graphicsPipeline);
}
inline static MpgxResult createVkGraphicsPipeline(
	VkDevice device,
	const VkGraphicsPipelineCreateData* createData,
	Framebuffer framebuffer,
	const char* name,
	GraphicsPipelineState state,
	OnGraphicsPipelineBind onBind,
	OnGraphicsPipelineUniformsSet onUniformsSet,
	OnGraphicsPipelineResize onResize,
	OnGraphicsPipelineDestroy onDestroy,
	void* handle,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(device);
	assert(createData);
	assert(framebuffer);
	assert(name);
	assert(onResize);
	assert(onDestroy);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	GraphicsPipeline graphicsPipelineInstance = calloc(1,
		sizeof(GraphicsPipeline_T));

	if (!graphicsPipelineInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	graphicsPipelineInstance->vk.framebuffer = framebuffer;
	graphicsPipelineInstance->vk.onBind = onBind;
	graphicsPipelineInstance->vk.onUniformsSet = onUniformsSet;
	graphicsPipelineInstance->vk.onResize = onResize;
	graphicsPipelineInstance->vk.onDestroy = onDestroy;
	graphicsPipelineInstance->vk.handle = handle;
	graphicsPipelineInstance->vk.state = state;
#ifndef NDEBUG
	graphicsPipelineInstance->vk.name = name;
#endif

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (!pipelineShaders)
	{
		destroyVkGraphicsPipeline(
			device,
			graphicsPipelineInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	for (size_t i = 0; i < shaderCount; i++)
		pipelineShaders[i] = shaders[i];

	graphicsPipelineInstance->vk.shaders = pipelineShaders;
	graphicsPipelineInstance->vk.shaderCount = shaderCount;

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
		destroyVkGraphicsPipeline(
			device,
			graphicsPipelineInstance,
			false);
		return vkToMpgxResult(vkResult);
	}

	graphicsPipelineInstance->vk.cache = cache;

	VkPipelineLayoutCreateInfo layoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		createData->setLayoutCount,
		createData->setLayouts,
		createData->pushConstantRangeCount,
		createData->pushConstantRanges,
	};

	VkPipelineLayout layout;

	vkResult = vkCreatePipelineLayout(
		device,
		&layoutCreateInfo,
		NULL,
		&layout);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkGraphicsPipeline(
			device,
			graphicsPipelineInstance,
			false);
		return vkToMpgxResult(vkResult);
	}

	graphicsPipelineInstance->vk.layout = layout;

	VkPipeline vkHandle;

	MpgxResult mpgxResult = createVkGraphicsPipelineHandle(
		device,
		framebuffer->vk.renderPass,
		cache,
		layout,
		shaders,
		shaderCount,
		state,
		framebuffer->vk.colorAttachmentCount,
		createData,
		&vkHandle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkGraphicsPipeline(
			device,
			graphicsPipelineInstance,
			false);
		return mpgxResult;
	}

	graphicsPipelineInstance->vk.vkHandle = vkHandle;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void bindVkGraphicsPipeline(
	VkCommandBuffer commandBuffer,
	GraphicsPipeline graphicsPipeline)
{
	assert(commandBuffer);
	assert(graphicsPipeline);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline->vk.vkHandle);

	if (graphicsPipeline->vk.onBind)
		graphicsPipeline->vk.onBind(graphicsPipeline);
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static bool getGlDrawMode(
	DrawMode drawMode,
	GLenum* glDrawMode)
{
	assert(drawMode < DRAW_MODE_COUNT);
	assert(glDrawMode);

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
	assert(polygonMode < POLYGON_MODE_COUNT);
	assert(_glPolygonMode);

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
	assert(cullMode < CULL_MODE_COUNT);
	assert(glCullMode);

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
	assert(blendFactor < BLEND_FACTOR_COUNT);
	assert(glBlendFactor);

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
	case SOURCE_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_COLOR;
		return true;
	case ONE_MINUS_SOURCE_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_SRC_COLOR;
		return true;
	case DESTINATION_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_DST_COLOR;
		return true;
	case ONE_MINUS_DESTINATION_COLOR_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_DST_COLOR;
		return true;
	case SOURCE_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_ALPHA;
		return true;
	case ONE_MINUS_SOURCE_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_ONE_MINUS_SRC_ALPHA;
		return true;
	case DESTINATION_ALPHA_BLEND_FACTOR:
		*glBlendFactor = GL_DST_ALPHA;
		return true;
	case ONE_MINUS_DESTINATION_ALPHA_BLEND_FACTOR:
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
	case SOURCE_ALPHA_SATURATE_BLEND_FACTOR:
		*glBlendFactor = GL_SRC_ALPHA_SATURATE;
		return true;
	}
}
inline static bool getGlBlendOperator(
	BlendOperator blendOperator,
	GLenum* glBlendOperator)
{
	assert(blendOperator < BLEND_OPERATOR_COUNT);
	assert(glBlendOperator);

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

inline static void destroyGlGraphicsPipeline(
	GraphicsPipeline graphicsPipeline,
	bool destroyShaders)
{
	if (!graphicsPipeline)
		return;

	makeGlWindowContextCurrent(
		graphicsPipeline->gl.framebuffer->gl.window);

	glDeleteProgram(graphicsPipeline->gl.glHandle);
	assertOpenGL();

	if (destroyShaders)
	{
		Shader* shaders = graphicsPipeline->gl.shaders;
		size_t shaderCount = graphicsPipeline->gl.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(graphicsPipeline->gl.shaders);
	free(graphicsPipeline);
}
inline static MpgxResult createGlGraphicsPipeline(
	Framebuffer framebuffer,
	const char* name,
	GraphicsPipelineState state,
	OnGraphicsPipelineBind onBind,
	OnGraphicsPipelineUniformsSet onUniformsSet,
	OnGraphicsPipelineResize onResize,
	OnGraphicsPipelineDestroy onDestroy,
	void* handle,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer);
	assert(name);
	assert(onResize);
	assert(onDestroy);
	assert(shaders);
	assert(shaderCount > 0);
	assert(graphicsPipeline);

	GraphicsPipeline graphicsPipelineInstance = calloc(1,
		sizeof(GraphicsPipeline_T));

	if (!graphicsPipelineInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	graphicsPipelineInstance->gl.framebuffer = framebuffer;
	graphicsPipelineInstance->gl.onBind = onBind;
	graphicsPipelineInstance->gl.onUniformsSet = onUniformsSet;
	graphicsPipelineInstance->gl.onResize = onResize;
	graphicsPipelineInstance->gl.onDestroy = onDestroy;
	graphicsPipelineInstance->gl.handle = handle;
	graphicsPipelineInstance->gl.state = state;
#ifndef NDEBUG
	graphicsPipelineInstance->gl.name = name;
#endif

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (!pipelineShaders)
	{
		destroyGlGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	graphicsPipelineInstance->gl.shaders = pipelineShaders;
	graphicsPipelineInstance->gl.shaderCount = shaderCount;

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

	if (state.cullFace)
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

	if (!result)
	{
		destroyGlGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	graphicsPipelineInstance->gl.drawMode = drawMode;
	graphicsPipelineInstance->gl.polygonMode = polygonMode;
	graphicsPipelineInstance->gl.cullMode = cullMode;
	graphicsPipelineInstance->gl.depthCompareOperator = depthCompareOperator;
	graphicsPipelineInstance->gl.srcColorBlendFactor = srcColorBlendFactor;
	graphicsPipelineInstance->gl.dstColorBlendFactor = dstColorBlendFactor;
	graphicsPipelineInstance->gl.srcAlphaBlendFactor = srcAlphaBlendFactor;
	graphicsPipelineInstance->gl.dstAlphaBlendFactor = dstAlphaBlendFactor;
	graphicsPipelineInstance->gl.colorBlendOperator = colorBlendOperator;
	graphicsPipelineInstance->gl.alphaBlendOperator = alphaBlendOperator;

	graphicsPipelineInstance->gl.frontFace =
		state.clockwiseFrontFace ? GL_CW : GL_CCW;

	Window window = framebuffer->gl.window;
	makeGlWindowContextCurrent(window);

	GLuint glHandle = glCreateProgram();
	graphicsPipelineInstance->gl.glHandle = glHandle;

	for (size_t i = 0; i < shaderCount; i++)
	{
		Shader shader = pipelineShaders[i] = shaders[i];

		glAttachShader(
			glHandle,
			shader->gl.handle);
	}

	glLinkProgram(glHandle);

	for (size_t i = 0; i < shaderCount; i++)
	{
		glDetachShader(
			glHandle,
			shaders[i]->gl.handle);
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
			char* infoLog = malloc(length * sizeof(char));

			if (!infoLog)
			{
				destroyGlGraphicsPipeline(
					graphicsPipelineInstance,
					false);
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			}

			glGetProgramInfoLog(
				glHandle,
				length,
				&length,
				(GLchar*)infoLog);

			printf("OpenGL program link error:\n%s", infoLog);
			free(infoLog);
		}

		assertOpenGL();

		destroyGlGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void bindGlGraphicsPipeline(
	GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline);

	Vec4I viewport = graphicsPipeline->gl.state.viewport;

	if (viewport.z + viewport.w > 0)
	{
		Vec2F depthRange =
			graphicsPipeline->gl.state.depthRange;

		glViewport(
			(GLint)viewport.x,
			(GLint)viewport.y,
			(GLsizei)viewport.z,
			(GLsizei)viewport.w);
		glDepthRange(
			depthRange.x,
			depthRange.y);
	}

	Vec4I scissor = graphicsPipeline->gl.state.scissor;

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
		graphicsPipeline->gl.polygonMode);

	if (graphicsPipeline->gl.state.cullFace)
	{
		glFrontFace(graphicsPipeline->gl.frontFace);
		glCullFace(graphicsPipeline->gl.cullMode);
		glEnable(GL_CULL_FACE);
	}
	else
	{
		glDisable(GL_CULL_FACE);
	}

	ColorComponent colorMask =
		graphicsPipeline->gl.state.colorComponentWriteMask;

	glColorMask(
		colorMask & RED_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & GREEN_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & BLUE_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE,
		colorMask & ALPHA_COLOR_COMPONENT ?
			GL_TRUE : GL_FALSE);

	if (graphicsPipeline->gl.state.testDepth)
	{
		if (graphicsPipeline->gl.state.clampDepth)
			glEnable(GL_DEPTH_CLAMP);
		else
			glDisable(GL_DEPTH_CLAMP);

		glDepthMask(
			graphicsPipeline->gl.state.writeDepth ?
			GL_TRUE : GL_FALSE);
		glDepthFunc(graphicsPipeline->gl.depthCompareOperator);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (graphicsPipeline->gl.state.enableDepthBias)
	{
		Vec2F depthBias =
			graphicsPipeline->gl.state.depthBias;

		glPolygonOffset(
			depthBias.y,
			depthBias.x);
		glEnable(GL_POLYGON_OFFSET_FILL);
	}
	else
	{
		glDisable(GL_POLYGON_OFFSET_FILL);
	}

	if (graphicsPipeline->gl.state.enableBlend)
	{
		Vec4F blendColor =
			graphicsPipeline->gl.state.blendColor;

		glBlendFuncSeparate(
			graphicsPipeline->gl.srcColorBlendFactor,
			graphicsPipeline->gl.dstColorBlendFactor,
			graphicsPipeline->gl.srcAlphaBlendFactor,
			graphicsPipeline->gl.dstAlphaBlendFactor);
		glBlendEquationSeparate(
			graphicsPipeline->gl.colorBlendOperator,
			graphicsPipeline->gl.alphaBlendOperator);
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

	if (graphicsPipeline->gl.state.restartPrimitive)
		glEnable(GL_PRIMITIVE_RESTART);
	else
		glDisable(GL_PRIMITIVE_RESTART);

	// TODO:
	glDisable(GL_STENCIL_TEST);

	glUseProgram(graphicsPipeline->gl.glHandle);
	assertOpenGL();

	if (graphicsPipeline->gl.onBind)
		graphicsPipeline->gl.onBind(graphicsPipeline);
}
inline static bool getGlUniformLocation(
	GLuint program,
	const GLchar* name,
	GLint* location)
{
	assert(program != GL_ZERO);
	assert(name);
	assert(location);

	GLint uniformLocation = glGetUniformLocation(
		program,
		name);

	if (uniformLocation == -1)
	{
		printf("Failed to get '%s' uniform location.\n", name);
		return false;
	}

	*location = uniformLocation;
	return true;
}
inline static GLuint getGlUniformBlockIndex(
	GLuint program,
	const GLchar* name,
	GLuint* blockIndex)
{
	assert(program != GL_ZERO);
	assert(name);
	assert(blockIndex);

	GLuint uniformBlockIndex = glGetUniformBlockIndex(
		program,
		name);

	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		printf("Failed to get '%s' uniform block index.\n", name);
		return false;
	}

	*blockIndex = uniformBlockIndex;
	return true;
}
#endif
