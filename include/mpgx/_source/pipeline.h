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

typedef struct BasePipeline_T
{
	Framebuffer framebuffer;
	PipelineState state;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineResize onResize;
	OnPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
#ifndef NDEBUG
	const char* name;
#endif
} BasePipeline_T;
typedef struct VkPipeline_T
{
	Framebuffer framebuffer;
	PipelineState state;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineResize onResize;
	OnPipelineDestroy onDestroy;
	void* handle;
	Shader* shaders;
	size_t shaderCount;
#ifndef NDEBUG
	const char* name;
#endif
#if MPGX_SUPPORT_VULKAN
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
#endif
} VkPipeline_T;
typedef struct GlPipeline_T
{
	Framebuffer framebuffer;
	PipelineState state;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;
	OnPipelineResize onResize;
	OnPipelineDestroy onDestroy;
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
} GlPipeline_T;
union Pipeline_T
{
	BasePipeline_T base;
	VkPipeline_T vk;
	GlPipeline_T gl;
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

inline static VkPipeline createVkPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	Shader* shaders,
	size_t shaderCount,
	PipelineState state,
	size_t colorAttachmentCount,
	const VkPipelineCreateInfo* createInfo)
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
inline static bool recreateVkPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	Pipeline pipeline,
	size_t colorAttachmentCount,
	const VkPipelineCreateInfo* createInfo)
{
	VkPipeline handle = createVkPipelineHandle(
		device,
		renderPass,
		pipeline->vk.cache,
		pipeline->vk.layout,
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

inline static void destroyVkPipeline(
	VkDevice device,
	Pipeline pipeline,
	bool destroyShaders)
{
	if (pipeline == NULL)
		return;

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

	if (destroyShaders == true)
	{
		Shader* shaders = pipeline->vk.shaders;
		size_t shaderCount = pipeline->vk.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(pipeline->vk.shaders);
	free(pipeline);
}
inline static Pipeline createVkPipeline(
	VkDevice device,
	const VkPipelineCreateInfo* createInfo,
	Framebuffer framebuffer,
	const char* name,
	PipelineState state,
	OnPipelineBind onBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineResize onResize,
	OnPipelineDestroy onDestroy,
	void* handle,
	Shader* shaders,
	size_t shaderCount)
{
	Pipeline pipeline = calloc(1, sizeof(Pipeline_T));

	if (pipeline == NULL)
		return NULL;

#ifndef NDEBUG
	pipeline->vk.name = name;
#endif
	pipeline->vk.framebuffer = framebuffer;
	pipeline->vk.state = state;
	pipeline->vk.onBind = onBind;
	pipeline->vk.onUniformsSet = onUniformsSet;
	pipeline->vk.onResize = onResize;
	pipeline->vk.onDestroy = onDestroy;
	pipeline->vk.handle = handle;

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (pipelineShaders == NULL)
	{
		destroyVkPipeline(
			device,
			pipeline,
			false);
		return NULL;
	}

	for (size_t i = 0; i < shaderCount; i++)
		pipelineShaders[i] = shaders[i];

	pipeline->vk.shaders = pipelineShaders;
	pipeline->vk.shaderCount = shaderCount;

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
		destroyVkPipeline(
			device,
			pipeline,
			false);
		return NULL;
	}

	pipeline->vk.cache = cache;

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
		destroyVkPipeline(
			device,
			pipeline,
			false);
		return NULL;
	}

	pipeline->vk.layout = layout;

	VkPipeline vkHandle = createVkPipelineHandle(
		device,
		framebuffer->vk.renderPass,
		cache,
		layout,
		shaders,
		shaderCount,
		state,
		framebuffer->vk.colorAttachmentCount,
		createInfo);

	if (vkHandle == NULL)
	{
		destroyVkPipeline(
			device,
			pipeline,
			false);
		return NULL;
	}

	pipeline->vk.vkHandle = vkHandle;
	return pipeline;
}

inline static void bindVkPipeline(
	VkCommandBuffer commandBuffer,
	Pipeline pipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.vkHandle);

	if (pipeline->vk.onBind != NULL)
		pipeline->vk.onBind(pipeline);
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

inline static void destroyGlPipeline(
	Pipeline pipeline,
	bool destroyShaders)
{
	if (pipeline == NULL)
		return;

	makeWindowContextCurrent(
		pipeline->gl.framebuffer->gl.window);

	glDeleteProgram(
		pipeline->gl.glHandle);
	assertOpenGL();

	if (destroyShaders == true)
	{
		Shader* shaders = pipeline->gl.shaders;
		size_t shaderCount = pipeline->gl.shaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(pipeline->gl.shaders);
	free(pipeline);
}
inline static Pipeline createGlPipeline(
	Framebuffer framebuffer,
	const char* name,
	PipelineState state,
	OnPipelineBind onBind,
	OnPipelineUniformsSet onUniformsSet,
	OnPipelineResize onResize,
	OnPipelineDestroy onDestroy,
	void* handle,
	Shader* shaders,
	size_t shaderCount)
{
	Pipeline pipeline = calloc(1, sizeof(Pipeline_T));

	if (pipeline == NULL)
		return NULL;

#ifndef NDEBUG
	pipeline->gl.name = name;
#endif
	pipeline->gl.framebuffer = framebuffer;
	pipeline->gl.state = state;
	pipeline->gl.onBind = onBind;
	pipeline->gl.onUniformsSet = onUniformsSet;
	pipeline->gl.onResize = onResize;
	pipeline->gl.onDestroy = onDestroy;
	pipeline->gl.handle = handle;

	Shader* pipelineShaders = malloc(
		shaderCount * sizeof(Shader));

	if (pipelineShaders == NULL)
	{
		destroyGlPipeline(
			pipeline,
			false);
		return NULL;
	}

	pipeline->gl.shaders = pipelineShaders;
	pipeline->gl.shaderCount = shaderCount;

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
		destroyGlPipeline(
			pipeline,
			false);
		return NULL;
	}

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

	GLenum frontFace =
		state.clockwiseFrontFace == true ?
		GL_CW : GL_CCW;

	pipeline->gl.frontFace = frontFace;

	Window window = framebuffer->gl.window;
	makeWindowContextCurrent(window);

	GLuint glHandle = glCreateProgram();
	pipeline->gl.glHandle = glHandle;

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
				destroyGlPipeline(
					pipeline,
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

		destroyGlPipeline(
			pipeline,
			false);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlPipeline(
			pipeline,
			false);
		return NULL;
	}

	return pipeline;
}

inline static void bindGlPipeline(
	Pipeline pipeline)
{
	Vec4U viewport = pipeline->gl.state.viewport;

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

	Vec4U scissor = pipeline->gl.state.scissor;

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

	if (pipeline->gl.onBind != NULL)
		pipeline->gl.onBind(pipeline);
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
