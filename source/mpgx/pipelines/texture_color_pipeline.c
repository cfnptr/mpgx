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

#include "mpgx/pipelines/texture_color_pipeline.h"
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
		sizeof(Vec3F) + sizeof(Vec2F),
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[2] = {
	{
		0,
		0,
		VK_FORMAT_R32G32B32_SFLOAT,
		0,
	},
	{
		1,
		0,
		VK_FORMAT_R32G32_SFLOAT,
		sizeof(Vec3F),
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

static void onVkBind(GraphicsPipeline graphicsPipeline)
{
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
static bool onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
	Handle handle = graphicsPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		VkDescriptorPool descriptorPool = createVkDescriptorPool(
			device,
			bufferCount);

		if (descriptorPool == NULL)
			return false;

		VkDescriptorSet* descriptorSets = createVkDescriptorSets(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			handle->vk.sampler->vk.handle,
			handle->vk.texture->vk.imageView);

		if (descriptorSets == NULL)
		{
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			return false;
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
	return true;
}
static void onVkDestroy(void* _handle)
{
	Handle handle = _handle;
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
inline static GraphicsPipeline createVkPipeline(
	Framebuffer framebuffer,
	VkSampler sampler,
	VkImageView imageView,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount)
{
	VkWindow vkWindow = getVkWindow(framebuffer->vk.window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
	{
		free(handle);
		return NULL;
	}

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

	VkDescriptorPool descriptorPool = createVkDescriptorPool(
		device,
		bufferCount);

	if (descriptorPool == NULL)
	{
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
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
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	handle->vk.descriptorSetLayout = descriptorSetLayout;
	handle->vk.descriptorPool = descriptorPool;
	handle->vk.descriptorSets = descriptorSets;
	handle->vk.bufferCount = bufferCount;

	return createGraphicsPipeline(
		framebuffer,
		TEXTURE_COLOR_PIPELINE_NAME,
		state,
		onVkBind,
		onVkUniformsSet,
		onVkResize,
		onVkDestroy,
		handle,
		&createData,
		shaders,
		shaderCount);
}
#endif

#if MPGX_SUPPORT_OPENGL
static void onGlBind(GraphicsPipeline graphicsPipeline)
{
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
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) + sizeof(Vec2F),
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) + sizeof(Vec2F),
		(const void*)sizeof(Vec3F));

	assertOpenGL();
}
static bool onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
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
	return true;
}
static void onGlDestroy(void* handle)
{
	free((Handle)handle);
}
inline static GraphicsPipeline createGlPipeline(
	Framebuffer framebuffer,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount)
{
	GraphicsPipeline pipeline = createGraphicsPipeline(
		framebuffer,
		TEXTURE_COLOR_PIPELINE_NAME,
		state,
		onGlBind,
		onGlUniformsSet,
		onGlResize,
		onGlDestroy,
		handle,
		NULL,
		shaders,
		shaderCount);

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
		destroyGraphicsPipeline(pipeline, false);
		return NULL;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.sizeLocation = sizeLocation;
	handle->gl.offsetLocation = offsetLocation;
	handle->gl.colorLocation = colorLocation;
	handle->gl.textureLocation = textureLocation;
	return pipeline;
}
#endif

GraphicsPipeline createTextureColorPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state)
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

	Handle handle = malloc(sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

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
			2);
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
			2);
#else
		abort();
#endif
	}
	else
	{
		abort();
	};
}
GraphicsPipeline createTextureColorPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	assert(framebuffer != NULL);

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
		true,
		true,
		true,
		true,
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

	return createTextureColorPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state);
}

Image getTextureColorPipelineTexture(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.texture;
}
Sampler getTextureColorPipelineSampler(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.sampler;
}

Mat4F getTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setTextureColorPipelineMvp(
	GraphicsPipeline textureColorPipeline,
	Mat4F mvp)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

Vec2F getTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.vpc.size;
}
void setTextureColorPipelineSize(
	GraphicsPipeline textureColorPipeline,
	Vec2F size)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	handle->base.vpc.size = size;
}

Vec2F getTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.vpc.offset;
}
void setTextureColorPipelineOffset(
	GraphicsPipeline textureColorPipeline,
	Vec2F offset)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp( textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	handle->base.vpc.offset = offset;
}

LinearColor getTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	return handle->base.fpc.color;
}
void setTextureColorPipelineColor(
	GraphicsPipeline textureColorPipeline,
	LinearColor color)
{
	assert(textureColorPipeline != NULL);
	assert(strcmp(textureColorPipeline->base.name,
		TEXTURE_COLOR_PIPELINE_NAME) == 0);
	Handle handle = textureColorPipeline->base.handle;
	handle->base.fpc.color = color;
}
