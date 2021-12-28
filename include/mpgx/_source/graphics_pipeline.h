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

typedef struct BaseGraphicsPipeline_T
{
	Framebuffer framebuffer;
	GraphicsPipelineState state;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
#ifndef NDEBUG
	const char* name;
#endif
} BaseGraphicsPipeline_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkGraphicsPipeline_T
{
	Framebuffer framebuffer;
	GraphicsPipelineState state;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
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
	GraphicsPipelineState state;
	OnGraphicsPipelineBind onBind;
	OnGraphicsPipelineUniformsSet onUniformsSet;
	OnGraphicsPipelineResize onResize;
	OnGraphicsPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
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

inline static VkPipeline createVkGraphicsPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	Shader* shaders,
	size_t shaderCount,
	GraphicsPipelineState state,
	size_t colorAttachmentCount,
	const VkGraphicsPipelineCreateData* createData)
{
	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (shaderStageCreateInfos == NULL)
		return NULL;

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

	bool dynamicViewport = state.viewport.z +
		state.viewport.w == 0;

	if (dynamicViewport == true)
	{
		dynamicStates[dynamicStateCount++] =
			VK_DYNAMIC_STATE_VIEWPORT;
	}

	bool dynamicScissor = state.scissor.z +
		state.scissor.w == 0;

	if (dynamicScissor == true)
	{
		dynamicStates[dynamicStateCount++] =
			VK_DYNAMIC_STATE_SCISSOR;
	}

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
		0,
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
inline static bool recreateVkGraphicsPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	GraphicsPipeline graphicsPipeline,
	size_t colorAttachmentCount,
	const VkGraphicsPipelineCreateData* createData)
{
	VkPipeline handle = createVkGraphicsPipelineHandle(
		device,
		renderPass,
		graphicsPipeline->vk.cache,
		graphicsPipeline->vk.layout,
		graphicsPipeline->vk.shaders,
		graphicsPipeline->vk.shaderCount,
		graphicsPipeline->vk.state,
		colorAttachmentCount,
		createData);

	if (handle == NULL)
		return false;

	vkDestroyPipeline(
		device,
		graphicsPipeline->vk.vkHandle,
		NULL);

	graphicsPipeline->vk.vkHandle = handle;
	return true;
}

inline static void destroyVkGraphicsPipeline(
	VkDevice device,
	GraphicsPipeline graphicsPipeline,
	bool destroyShaders)
{
	if (graphicsPipeline == NULL)
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

	if (destroyShaders == true)
	{
		Shader* shaders = graphicsPipeline->vk.shaders;
		size_t shaderCount = graphicsPipeline->vk.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(graphicsPipeline->vk.shaders);
	free(graphicsPipeline);
}
inline static GraphicsPipeline createVkGraphicsPipeline(
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
	size_t shaderCount)
{
	GraphicsPipeline graphicsPipeline = calloc(1,
		sizeof(GraphicsPipeline_T));

	if (graphicsPipeline == NULL)
		return NULL;

#ifndef NDEBUG
	graphicsPipeline->vk.name = name;
#endif
	graphicsPipeline->vk.framebuffer = framebuffer;
	graphicsPipeline->vk.state = state;
	graphicsPipeline->vk.onBind = onBind;
	graphicsPipeline->vk.onUniformsSet = onUniformsSet;
	graphicsPipeline->vk.onResize = onResize;
	graphicsPipeline->vk.onDestroy = onDestroy;
	graphicsPipeline->vk.handle = handle;

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (pipelineShaders == NULL)
	{
		destroyVkGraphicsPipeline(
			device,
			graphicsPipeline,
			false);
		return NULL;
	}

	for (size_t i = 0; i < shaderCount; i++)
		pipelineShaders[i] = shaders[i];

	graphicsPipeline->vk.shaders = pipelineShaders;
	graphicsPipeline->vk.shaderCount = shaderCount;

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
			graphicsPipeline,
			false);
		return NULL;
	}

	graphicsPipeline->vk.cache = cache;

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
			graphicsPipeline,
			false);
		return NULL;
	}

	graphicsPipeline->vk.layout = layout;

	VkPipeline vkHandle = createVkGraphicsPipelineHandle(
		device,
		framebuffer->vk.renderPass,
		cache,
		layout,
		shaders,
		shaderCount,
		state,
		framebuffer->vk.colorAttachmentCount,
		createData);

	if (vkHandle == NULL)
	{
		destroyVkGraphicsPipeline(
			device,
			graphicsPipeline,
			false);
		return NULL;
	}

	graphicsPipeline->vk.vkHandle = vkHandle;
	return graphicsPipeline;
}

inline static void bindVkGraphicsPipeline(
	VkCommandBuffer commandBuffer,
	GraphicsPipeline graphicsPipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline->vk.vkHandle);

	if (graphicsPipeline->vk.onBind != NULL)
		graphicsPipeline->vk.onBind(graphicsPipeline);
}
#endif

#if MPGX_SUPPORT_OPENGL
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

inline static void destroyGlGraphicsPipeline(
	GraphicsPipeline graphicsPipeline,
	bool destroyShaders)
{
	if (graphicsPipeline == NULL)
		return;

	makeWindowContextCurrent(
		graphicsPipeline->gl.framebuffer->gl.window);

	glDeleteProgram(
		graphicsPipeline->gl.glHandle);
	assertOpenGL();

	if (destroyShaders == true)
	{
		Shader* shaders = graphicsPipeline->gl.shaders;
		size_t shaderCount = graphicsPipeline->gl.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(graphicsPipeline->gl.shaders);
	free(graphicsPipeline);
}
inline static GraphicsPipeline createGlGraphicsPipeline(
	Framebuffer framebuffer,
	const char* name,
	GraphicsPipelineState state,
	OnGraphicsPipelineBind onBind,
	OnGraphicsPipelineUniformsSet onUniformsSet,
	OnGraphicsPipelineResize onResize,
	OnGraphicsPipelineDestroy onDestroy,
	void* handle,
	Shader* shaders,
	size_t shaderCount)
{
	GraphicsPipeline graphicsPipeline = calloc(1,
		sizeof(GraphicsPipeline_T));

	if (graphicsPipeline == NULL)
		return NULL;

#ifndef NDEBUG
	graphicsPipeline->gl.name = name;
#endif
	graphicsPipeline->gl.framebuffer = framebuffer;
	graphicsPipeline->gl.state = state;
	graphicsPipeline->gl.onBind = onBind;
	graphicsPipeline->gl.onUniformsSet = onUniformsSet;
	graphicsPipeline->gl.onResize = onResize;
	graphicsPipeline->gl.onDestroy = onDestroy;
	graphicsPipeline->gl.handle = handle;

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (pipelineShaders == NULL)
	{
		destroyGlGraphicsPipeline(
			graphicsPipeline,
			false);
		return NULL;
	}

	graphicsPipeline->gl.shaders = pipelineShaders;
	graphicsPipeline->gl.shaderCount = shaderCount;

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
		destroyGlGraphicsPipeline(
			graphicsPipeline,
			false);
		return NULL;
	}

	graphicsPipeline->gl.drawMode = drawMode;
	graphicsPipeline->gl.polygonMode = polygonMode;
	graphicsPipeline->gl.cullMode = cullMode;
	graphicsPipeline->gl.depthCompareOperator = depthCompareOperator;
	graphicsPipeline->gl.srcColorBlendFactor = srcColorBlendFactor;
	graphicsPipeline->gl.dstColorBlendFactor = dstColorBlendFactor;
	graphicsPipeline->gl.srcAlphaBlendFactor = srcAlphaBlendFactor;
	graphicsPipeline->gl.dstAlphaBlendFactor = dstAlphaBlendFactor;
	graphicsPipeline->gl.colorBlendOperator = colorBlendOperator;
	graphicsPipeline->gl.alphaBlendOperator = alphaBlendOperator;

	GLenum frontFace =
		state.clockwiseFrontFace == true ?
		GL_CW : GL_CCW;

	graphicsPipeline->gl.frontFace = frontFace;

	Window window = framebuffer->gl.window;
	makeWindowContextCurrent(window);

	GLuint glHandle = glCreateProgram();
	graphicsPipeline->gl.glHandle = glHandle;

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
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				destroyGlGraphicsPipeline(
					graphicsPipeline,
					false);
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

		destroyGlGraphicsPipeline(
			graphicsPipeline,
			false);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlGraphicsPipeline(
			graphicsPipeline,
			false);
		return NULL;
	}

	return graphicsPipeline;
}

inline static void bindGlGraphicsPipeline(
	GraphicsPipeline graphicsPipeline)
{
	Vec4U viewport = graphicsPipeline->gl.state.viewport;

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

	Vec4U scissor = graphicsPipeline->gl.state.scissor;

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

	if (graphicsPipeline->gl.state.cullFace == true)
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

	if (graphicsPipeline->gl.state.testDepth == true)
	{
		if (graphicsPipeline->gl.state.clampDepth == true)
			glEnable(GL_DEPTH_CLAMP);
		else
			glDisable(GL_DEPTH_CLAMP);

		glDepthMask(
			graphicsPipeline->gl.state.writeDepth == true ?
			GL_TRUE : GL_FALSE);
		glDepthFunc(graphicsPipeline->gl.depthCompareOperator);
		glEnable(GL_DEPTH_TEST);
	}
	else
	{
		glDisable(GL_DEPTH_TEST);
	}

	if (graphicsPipeline->gl.state.enableDepthBias == true)
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

	if (graphicsPipeline->gl.state.enableBlend == true)
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

	if (graphicsPipeline->gl.state.restartPrimitive == true)
		glEnable(GL_PRIMITIVE_RESTART);
	else
		glDisable(GL_PRIMITIVE_RESTART);

	// TODO:
	glDisable(GL_STENCIL_TEST);

	glUseProgram(graphicsPipeline->gl.glHandle);
	assertOpenGL();

	if (graphicsPipeline->gl.onBind != NULL)
		graphicsPipeline->gl.onBind(graphicsPipeline);
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
#endif
