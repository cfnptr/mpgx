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

#include "mpgx/pipelines/bloom_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

typedef struct FragmentPushConstants
{
	LinearColor threshold;
} FragmentPushConstants;
typedef struct BaseHandle
{
	Window window;
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Window window;
	Image buffer;
	Sampler sampler;
	FragmentPushConstants fpc;
	GLint thresholdLocation;
	GLint bufferLocation;
} GlHandle;
#endif
typedef union Handle_T
{
	BaseHandle base;
#if MPGX_SUPPORT_VULKAN
	VkHandle vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlHandle gl;
#endif
} Handle_T;

typedef Handle_T* Handle;

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(Vec2F) * 2,
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		sizeof(Vec2F),
	},
};
static const VkPushConstantRange pushConstantRanges[1] = {
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(FragmentPushConstants),
	},
};

inline static MpgxResult createVkDescriptorPoolInstance(
	VkDevice device,
	uint32_t bufferCount,
	VkDescriptorPool* descriptorPool)
{
	assert(device != NULL);
	assert(bufferCount != 0);
	assert(descriptorPool != NULL);

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferCount,
		},
	};

	VkDescriptorPool descriptorPoolInstance;

	MpgxResult mpgxResult = createVkDescriptorPool(
		device,
		bufferCount,
		descriptorPoolSizes,
		1,
		&descriptorPoolInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	*descriptorPool = descriptorPoolInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDescriptorSetArray(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	VkImageView bufferImageView,
	VkSampler sampler,
	VkDescriptorSet** descriptorSets)
{
	assert(device != NULL);
	assert(descriptorSetLayout != NULL);
	assert(descriptorPool != NULL);
	assert(bufferCount != 0);
	assert(bufferImageView != NULL);
	assert(sampler != NULL);
	assert(descriptorSets != NULL);

	VkDescriptorSet* descriptorSetArray;

	MpgxResult mpgxResult = allocateVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		&descriptorSetArray);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkDescriptorImageInfo bufferDescriptorImageInfos[1] = {
			{
				sampler,
				bufferImageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
		};

		VkWriteDescriptorSet writeDescriptorSets[1] = {
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				NULL,
				descriptorSetArray[i],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				bufferDescriptorImageInfos,
				NULL,
				NULL,
			},
		};

		vkUpdateDescriptorSets(
			device,
			1,
			writeDescriptorSets,
			0,
			NULL);
	}

	*descriptorSets = descriptorSetArray;
	return SUCCESS_MPGX_RESULT;
}

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;

	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		graphicsPipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSets[bufferIndex],
		0,
		NULL);
}
static void onVkUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);

	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		graphicsPipeline->vk.layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(FragmentPushConstants),
		&handle->vk.fpc);
}
static MpgxResult onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
	assert(graphicsPipeline != NULL);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData != NULL);

	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		VkDescriptorPool descriptorPool;

		MpgxResult mpgxResult = createVkDescriptorPoolInstance(
			device,
			bufferCount,
			&descriptorPool);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
			return mpgxResult;

		VkDescriptorSet* descriptorSets;

		mpgxResult = createVkDescriptorSetArray(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			handle->vk.buffer->vk.imageView,
			handle->vk.sampler->vk.handle,
			&descriptorSets);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			return mpgxResult;
		}

		free(handle->vk.descriptorSets);

		vkDestroyDescriptorPool(
			device,
			handle->vk.descriptorPool,
			NULL);

		handle->vk.descriptorPool = descriptorPool;
		handle->vk.descriptorSets = descriptorSets;
		handle->vk.bufferCount = bufferCount;
	}

	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = graphicsPipeline->vk.state.viewport.z +
		graphicsPipeline->vk.state.viewport.w == 0;
	if (dynamic == false)
		graphicsPipeline->vk.state.viewport = size;

	dynamic = graphicsPipeline->vk.state.scissor.z +
		graphicsPipeline->vk.state.scissor.w == 0;
	if (dynamic == false)
		graphicsPipeline->vk.state.scissor = size;

	VkGraphicsPipelineCreateData _createData = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
	return SUCCESS_MPGX_RESULT;
}
static void onVkDestroy(void* _handle)
{
	Handle handle = _handle;

	if (handle == NULL)
		return;

	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkDevice device = vkWindow->device;

	free(handle->vk.descriptorSets);
	vkDestroyDescriptorPool(
		device,
		handle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		handle->vk.descriptorSetLayout,
		NULL);
	free(handle);
}
inline static MpgxResult createVkPipeline(
	Framebuffer framebuffer,
	VkImageView imageView,
	VkSampler sampler,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer != NULL);
	assert(imageView != NULL);
	assert(sampler != NULL);
	assert(state != NULL);
	assert(handle != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);
	assert(graphicsPipeline != NULL);

	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL,
		},
	};

	VkDescriptorSetLayout descriptorSetLayout;

	MpgxResult mpgxResult = createVkDescriptorSetLayout(
		device,
		descriptorSetLayoutBindings,
		1,
		&descriptorSetLayout);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(handle);
		return mpgxResult;
	}

	handle->vk.descriptorSetLayout = descriptorSetLayout;

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferCount,
		},
	};

	VkDescriptorPool descriptorPool;

	mpgxResult = createVkDescriptorPool(
		device,
		bufferCount,
		descriptorPoolSizes,
		1,
		&descriptorPool);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(handle);
		return mpgxResult;
	}

	handle->vk.descriptorPool = descriptorPool;

	VkDescriptorSet* descriptorSets;

	mpgxResult = createVkDescriptorSetArray(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		imageView,
		sampler,
		&descriptorSets);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(handle);
		return mpgxResult;
	}

	handle->vk.descriptorSets = descriptorSets;
	handle->vk.bufferCount = bufferCount;

	GraphicsPipeline graphicsPipelineInstance;

	mpgxResult = createGraphicsPipeline(
		framebuffer,
		BLOOM_PIPELINE_NAME,
		state,
		onVkBind,
		onVkUniformsSet,
		onVkResize,
		onVkDestroy,
		handle,
		&createData,
		shaders,
		shaderCount,
		&graphicsPipelineInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onVkDestroy(handle);
		return mpgxResult;
	}

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
static void onGlBind(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->gl.handle;

	glUniform1i(
		handle->gl.bufferLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		handle->gl.buffer->gl.handle);
	glBindSampler(
		0,
		handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->gl.handle;

	glUniform4fv(
		handle->gl.thresholdLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.threshold);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		(const void*)sizeof(Vec2F));

	assertOpenGL();
}
static MpgxResult onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
	assert(graphicsPipeline != NULL);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData == NULL);

	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = graphicsPipeline->gl.state.viewport.z +
		graphicsPipeline->gl.state.viewport.w == 0;
	if (dynamic == false)
		graphicsPipeline->gl.state.viewport = size;

	dynamic = graphicsPipeline->gl.state.scissor.z +
		graphicsPipeline->gl.state.scissor.w == 0;
	if (dynamic == false)
		graphicsPipeline->gl.state.scissor = size;
	return SUCCESS_MPGX_RESULT;
}
static void onGlDestroy(void* handle)
{
	free((Handle)handle);
}
inline static MpgxResult createGlPipeline(
	Framebuffer framebuffer,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer != NULL);
	assert(state != NULL);
	assert(handle != NULL);
	assert(shaders != NULL);
	assert(shaderCount != 0);
	assert(graphicsPipeline != NULL);

	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		BLOOM_PIPELINE_NAME,
		state,
		NULL,
		onGlUniformsSet,
		onGlResize,
		onGlDestroy,
		handle,
		NULL,
		shaders,
		shaderCount,
		&graphicsPipelineInstance);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		onGlDestroy(handle);
		return mpgxResult;
	}

	GLuint glHandle = graphicsPipelineInstance->gl.glHandle;

	GLint thresholdLocation, bufferLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_Threshold",
		&thresholdLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Buffer",
		&bufferLocation);

	if (result == false)
	{
		destroyGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.thresholdLocation = thresholdLocation;
	handle->gl.bufferLocation = bufferLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createBloomPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* bloomPipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(buffer != NULL);
	assert(sampler != NULL);
	assert(state != NULL);
	assert(bloomPipeline != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(buffer->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.buffer = buffer;
	handle->base.sampler = sampler;
	handle->base.fpc.threshold = whiteLinearColor;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkPipeline(
			framebuffer,
			buffer->vk.imageView,
			sampler->vk.handle,
			state,
			handle,
			shaders,
			2,
			bloomPipeline);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
#if MPGX_SUPPORT_OPENGL
		return createGlPipeline(
			framebuffer,
			state,
			handle,
			shaders,
			2,
			bloomPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
MpgxResult createBloomPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image buffer,
	Sampler sampler,
	GraphicsPipeline* bloomPipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(vertexShader != NULL);
	assert(buffer != NULL);
	assert(sampler != NULL);
	assert(bloomPipeline != NULL);

	Vec2U framebufferSize =
		framebuffer->base.size;
	Vec4U size = vec4U(0, 0,
		framebufferSize.x,
		framebufferSize.y);

	GraphicsPipelineState state = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		ALL_COLOR_COMPONENT,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	return createBloomPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		buffer,
		sampler,
		&state,
		bloomPipeline);
}

Image getBloomPipelineBuffer(
	GraphicsPipeline bloomPipeline)
{
	assert(bloomPipeline != NULL);
	assert(strcmp(bloomPipeline->base.name,
		BLOOM_PIPELINE_NAME) == 0);
	Handle handle = bloomPipeline->base.handle;
	return handle->base.buffer;
}
Sampler getBloomPipelineSampler(
	GraphicsPipeline bloomPipeline)
{
	assert(bloomPipeline != NULL);
	assert(strcmp(bloomPipeline->base.name,
		BLOOM_PIPELINE_NAME) == 0);
	Handle handle = bloomPipeline->base.handle;
	return handle->base.sampler;
}

LinearColor getBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline)
{
	assert(bloomPipeline != NULL);
	assert(strcmp(bloomPipeline->base.name,
		BLOOM_PIPELINE_NAME) == 0);
	Handle handle = bloomPipeline->base.handle;
	return handle->base.fpc.threshold;
}
void setBloomPipelineThreshold(
	GraphicsPipeline bloomPipeline,
	LinearColor threshold)
{
	assert(bloomPipeline != NULL);
	assert(strcmp(bloomPipeline->base.name,
		BLOOM_PIPELINE_NAME) == 0);
	Handle handle = bloomPipeline->base.handle;
	handle->base.fpc.threshold = threshold;
}
