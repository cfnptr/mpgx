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

#include "mpgx/pipelines/texspr_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
	Vec2F size;
	Vec2F offset;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	LinearColor color;
} FragmentPushConstants;
typedef struct BasePipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkImageView imageView;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
#endif
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint sizeLocation;
	GLint offsetLocation;
	GLint colorLocation;
	GLint textureLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

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
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
	},
};

inline static VkDescriptorSetLayout createVkDescriptorSetLayout(
	VkDevice device)
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			1,
			VK_SHADER_STAGE_FRAGMENT_BIT,
			NULL,
		},
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		1,
		descriptorSetLayoutBindings,
	};

	VkDescriptorSetLayout descriptorSetLayout;

	VkResult result = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayout);

	if(result != VK_SUCCESS)
		return NULL;

	return descriptorSetLayout;
}
inline static VkDescriptorPool createVkDescriptorPool(
	VkDevice device,
	uint32_t bufferCount)
{
	VkDescriptorPoolSize descriptorPoolSizes[1] = {
		{
			VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
			bufferCount,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		bufferCount,
		1,
		descriptorPoolSizes,
	};

	VkDescriptorPool descriptorPool;

	VkResult result = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPool);

	if (result != VK_SUCCESS)
		return NULL;

	return descriptorPool;
}
inline static VkDescriptorSet* createVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	VkSampler sampler,
	VkImageView imageView)
{
	VkDescriptorSetLayout* descriptorSetLayouts = malloc(
		bufferCount * sizeof(VkDescriptorSetLayout));

	if (descriptorSetLayouts == NULL)
		return NULL;

	for (uint32_t i = 0; i < bufferCount; i++)
		descriptorSetLayouts[i] = descriptorSetLayout;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo ={
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		bufferCount,
		descriptorSetLayouts,
	};

	VkDescriptorSet* descriptorSets = malloc(
		bufferCount * sizeof(VkDescriptorSet));

	if (descriptorSets == NULL)
	{
		free(descriptorSetLayouts);
		return NULL;
	}

	VkResult result = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		descriptorSets);

	free(descriptorSetLayouts);

	if (result != VK_SUCCESS)
	{
		free(descriptorSets);
		return NULL;
	}

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkDescriptorImageInfo descriptorImageInfos[1] = {
			{
				sampler,
				imageView,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			},
		};
		VkWriteDescriptorSet writeDescriptorSets[1] = {
			{
				VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
				NULL,
				descriptorSets[i],
				0,
				0,
				1,
				VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
				descriptorImageInfos,
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

	return descriptorSets;
}

static void onVkHandleDestroy(void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkDevice device = vkWindow->device;

	free(pipelineHandle->vk.descriptorSets);
	vkDestroyImageView(
		device,
		pipelineHandle->vk.imageView,
		NULL);
	vkDestroyDescriptorPool(
		device,
		pipelineHandle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		pipelineHandle->vk.descriptorSetLayout,
		NULL);
	free(pipelineHandle);
}
static void onVkUniformsSet(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = pipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&pipelineHandle->vk.vpc);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
		sizeof(FragmentPushConstants),
		&pipelineHandle->vk.fpc);
}
static void onVkHandleBind(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;

	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.layout,
		0,
		1,
		&pipelineHandle->vk.descriptorSets[bufferIndex],
		0,
		NULL);
}
static bool onVkHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	PipelineHandle* pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != pipelineHandle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		VkDescriptorPool descriptorPool = createVkDescriptorPool(
			device,
			bufferCount);

		if (descriptorPool == NULL)
			return false;

		VkDescriptorSet* descriptorSets = createVkDescriptorSets(
			device,
			pipelineHandle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			pipelineHandle->vk.sampler->vk.handle,
			pipelineHandle->vk.imageView);

		if (descriptorSets == NULL)
		{
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			return false;
		}

		free(pipelineHandle->vk.descriptorSets);

		vkDestroyDescriptorPool(
			device,
			pipelineHandle->vk.descriptorPool,
			NULL);

		pipelineHandle->vk.descriptorPool = descriptorPool;
		pipelineHandle->vk.descriptorSets = descriptorSets;
		pipelineHandle->vk.bufferCount = bufferCount;
	}

	Vec4I size = vec4I(0, 0,
		(int32_t)newSize.x,
		(int32_t)newSize.y);
	pipeline->vk.state.viewport = size;
	pipeline->vk.state.scissor = size;

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&pipelineHandle->vk.descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
	return true;
}
inline static Pipeline createVkHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	VkSampler sampler,
	Image image,
	const PipelineState* state,
	PipelineHandle* pipelineHandle)
{
	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
		return NULL;

	VkPipelineCreateInfo createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	Pipeline pipeline = createPipeline(
		framebuffer,
		TEXSPR_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onVkHandleDestroy,
		onVkHandleBind,
		onVkUniformsSet,
		onVkHandleResize,
		pipelineHandle,
		&createInfo);

	if (pipeline == NULL)
	{
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		return NULL;
	}

	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	VkDescriptorPool descriptorPool = createVkDescriptorPool(
		device,
		bufferCount);

	if (descriptorPool == NULL)
	{
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		return NULL;
	}

	VkImageView imageView = createVkImageView(
		device,
		image->vk.handle,
		image->vk.vkFormat,
		image->vk.vkAspect);

	if (imageView == NULL)
	{
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
	}

	VkDescriptorSet* descriptorSets = createVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		sampler,
		imageView);

	if (descriptorSets == NULL)
	{
		vkDestroyImageView(
			device,
			imageView,
			NULL);
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		destroyPipeline(
			pipeline,
			false);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		return NULL;
	}

	pipelineHandle->vk.descriptorSetLayout = descriptorSetLayout;
	pipelineHandle->vk.descriptorPool = descriptorPool;
	pipelineHandle->vk.imageView = imageView;
	pipelineHandle->vk.descriptorSets = descriptorSets;
	pipelineHandle->vk.bufferCount = bufferCount;
	return pipeline;
}
#endif

static void onGlHandleDestroy(void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
}
static void onGlHandleBind(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->gl.handle;

	glUniform1i(
		pipelineHandle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		pipelineHandle->gl.texture->gl.handle);
	glBindSampler(
		0,
		pipelineHandle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle* pipelineHandle = pipeline->gl.handle;

	glUniformMatrix4fv(
		pipelineHandle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.vpc.mvp);
	glUniform2fv(
		pipelineHandle->gl.sizeLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.vpc.size);
	glUniform2fv(
		pipelineHandle->gl.offsetLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.vpc.offset);
	glUniform4fv(
		pipelineHandle->gl.colorLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.fpc.color);

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
static bool onGlHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	Vec4I size = vec4I(0, 0,
		(int32_t)newSize.x,
		(int32_t)newSize.y);
	pipeline->gl.state.viewport = size;
	pipeline->gl.state.scissor = size;
	return true;
}
inline static Pipeline createGlHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* pipelineHandle)
{
	Pipeline pipeline = createPipeline(
		framebuffer,
		TEXSPR_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onGlHandleDestroy,
		onGlHandleBind,
		onGlUniformsSet,
		onGlHandleResize,
		pipelineHandle,
		NULL);

	if (pipeline == NULL)
		return NULL;

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation, sizeLocation, offsetLocation,
		colorLocation, textureLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Size",
		&sizeLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Offset",
		&offsetLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Texture",
		&textureLocation);

	if (result == false)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.sizeLocation = sizeLocation;
	pipelineHandle->gl.offsetLocation = offsetLocation;
	pipelineHandle->gl.colorLocation = colorLocation;
	pipelineHandle->gl.textureLocation = textureLocation;
	return pipeline;
}

Pipeline createExtTexSprPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(texture != NULL);
	assert(sampler != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(texture->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	PipelineHandle* pipelineHandle = malloc(
		sizeof(PipelineHandle));

	if (pipelineHandle == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Window window = framebuffer->base.window;
	GraphicsAPI api = getWindowGraphicsAPI(window);

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		pipeline = createVkHandle(
			framebuffer,
			shaders,
			2,
			sampler->vk.handle,
			texture,
			state,
			pipelineHandle);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipeline = createGlHandle(
			framebuffer,
			shaders,
			2,
			state,
			pipelineHandle);
	}
	else
	{
		abort();
	}

	if (pipeline == NULL)
	{
		free(pipelineHandle);
		return NULL;
	}

	pipelineHandle->base.window = window;
	pipelineHandle->base.texture = texture;
	pipelineHandle->base.sampler = sampler;
	pipelineHandle->base.vpc.mvp = identMat4F;
	pipelineHandle->base.vpc.size = oneVec2F;
	pipelineHandle->base.vpc.offset = zeroVec2F;
	pipelineHandle->base.fpc.color = whiteLinearColor;
	return pipeline;
}
Pipeline createTexSprPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	assert(framebuffer != NULL);

	Vec2U framebufferSize =
		framebuffer->base.size;
	Vec4I size = vec4I(0, 0,
		(int32_t)framebufferSize.x,
		(int32_t)framebufferSize.y);

	PipelineState state = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		ALL_COLOR_COMPONENT,
		SRC_ALPHA_BLEND_FACTOR,
		ONE_MINUS_SRC_ALPHA_BLEND_FACTOR,
		ONE_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		true,
		true,
		true,
		true,
		false,
		false,
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		size,
		defaultDepthRange,
		defaultDepthBias,
		defaultBlendColor,
	};

	return createExtTexSprPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state);
}

Image getTexSprPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.texture;
}
Sampler getTexSprPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.sampler;
}

Mat4F getTexSprPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.mvp;
}
void setTexSprPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.mvp = mvp;
}

Vec2F getTexSprPipelineSize(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.size;
}
void setTexSprPipelineSize(
	Pipeline pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.size = size;
}

Vec2F getTexSprPipelineOffset(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.offset;
}
void setTexSprPipelineOffset(
	Pipeline pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.offset = offset;
}

LinearColor getTexSprPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.fpc.color;
}
void setTexSprPipelineColor(
	Pipeline pipeline,
	LinearColor color)
{
	assert(pipeline != NULL);
	assertLinearColor(color);
	assert(strcmp(
		pipeline->base.name,
		TEXSPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.fpc.color = color;
}
