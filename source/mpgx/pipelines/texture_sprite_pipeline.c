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

#include "mpgx/pipelines/texture_sprite_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"
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
typedef struct BaseHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
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
	Image texture;
	Sampler sampler;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint sizeLocation;
	GLint offsetLocation;
	GLint colorLocation;
	GLint textureLocation;
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
	VkSampler sampler,
	VkImageView imageView,
	VkDescriptorSet** descriptorSets)
{
	assert(device != NULL);
	assert(descriptorSetLayout != NULL);
	assert(descriptorPool != NULL);
	assert(bufferCount != 0);
	assert(sampler != NULL);
	assert(imageView != NULL);
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
				descriptorSetArray[i],
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
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = graphicsPipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&handle->vk.vpc);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(VertexPushConstants),
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
			handle->vk.sampler->vk.handle,
			handle->vk.texture->vk.imageView,
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
		2,
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
	VkSampler sampler,
	VkImageView imageView,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount,
	GraphicsPipeline* graphicsPipeline)
{
	assert(framebuffer != NULL);
	assert(sampler != NULL);
	assert(imageView != NULL);
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

	if(mpgxResult != SUCCESS_MPGX_RESULT)
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
		2,
		pushConstantRanges,
	};

	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	VkDescriptorPool descriptorPool;

	mpgxResult = createVkDescriptorPoolInstance(
		device,
		bufferCount,
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
		sampler,
		imageView,
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
		TEXTURE_SPRITE_PIPELINE_NAME,
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
		handle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		handle->gl.texture->gl.handle);
	glBindSampler(
		0,
		handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.mvp);
	glUniform2fv(
		handle->gl.sizeLocation,
		1,
		(const GLfloat*)&handle->gl.vpc.size);
	glUniform2fv(
		handle->gl.offsetLocation,
		1,
		(const GLfloat*)&handle->gl.vpc.offset);
	glUniform4fv(
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.color);

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
	assert(createData != NULL);

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
		TEXTURE_SPRITE_PIPELINE_NAME,
		state,
		onGlBind,
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
		destroyGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.sizeLocation = sizeLocation;
	handle->gl.offsetLocation = offsetLocation;
	handle->gl.colorLocation = colorLocation;
	handle->gl.textureLocation = textureLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createTextureSpritePipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* textureSpritePipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(texture != NULL);
	assert(sampler != NULL);
	assert(state != NULL);
	assert(textureSpritePipeline != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);
	assert(texture->base.window == framebuffer->base.window);
	assert(sampler->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.texture = texture;
	handle->base.sampler = sampler;
	handle->base.vpc.mvp = identMat4F;
	handle->base.vpc.size = oneVec2F;
	handle->base.vpc.offset = zeroVec2F;
	handle->base.fpc.color = whiteLinearColor;

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
			sampler->vk.handle,
			texture->vk.imageView,
			state,
			handle,
			shaders,
			2,
			textureSpritePipeline);
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
			textureSpritePipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
MpgxResult createTextureSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	GraphicsPipeline* textureSpritePipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(texture != NULL);
	assert(sampler != NULL);
	assert(textureSpritePipeline != NULL);

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

	return createTextureSpritePipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state,
		textureSpritePipeline);
}

Image getTextureSpritePipelineTexture(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.texture;
}
Sampler getTextureSpritePipelineSampler(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.sampler;
}

Mat4F getTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setTextureSpritePipelineMvp(
	GraphicsPipeline textureSpritePipeline,
	Mat4F mvp)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

Vec2F getTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.vpc.size;
}
void setTextureSpritePipelineSize(
	GraphicsPipeline textureSpritePipeline,
	Vec2F size)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	handle->base.vpc.size = size;
}

Vec2F getTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.vpc.offset;
}
void setTextureSpritePipelineOffset(
	GraphicsPipeline textureSpritePipeline,
	Vec2F offset)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	handle->base.vpc.offset = offset;
}

LinearColor getTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	return handle->base.fpc.color;
}
void setTextureSpritePipelineColor(
	GraphicsPipeline textureSpritePipeline,
	LinearColor color)
{
	assert(textureSpritePipeline != NULL);
	assert(strcmp(textureSpritePipeline->base.name,
		TEXTURE_SPRITE_PIPELINE_NAME) == 0);
	Handle handle = textureSpritePipeline->base.handle;
	handle->base.fpc.color = color;
}
