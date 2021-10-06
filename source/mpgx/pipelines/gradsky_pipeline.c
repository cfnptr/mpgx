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

#include "mpgx/pipelines/gradsky_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

struct GradSkyAmbient
{
	Vec4F* colors;
	size_t count;
};

typedef struct VkPipelineHandle
{
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F sunDir;
	Vec4F sunColor;
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
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F sunDir;
	Vec4F sunColor;
	GLint mvpLocation;
	GLint sunDirLocation;
	GLint sunColorLocation;
	GLint textureLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

GradSkyAmbient createGradSkyAmbient(
	ImageData gradient)
{
	assert(gradient != NULL);
	assert(getImageDataChannelCount(gradient) == 4);

	Vec2U size = getImageDataSize(gradient);

	Vec4F* colors = malloc(
		sizeof(Vec4F) * size.x);

	if (colors == NULL)
		return NULL;

	const uint8_t* pixels = getImageDataPixels(gradient);

	for (uint32_t x = 0; x < size.x; x++)
	{
		Vec4F color = zeroVec4F();

		for (uint32_t y = 0; y < size.y; y++)
		{
			size_t index = (y * size.x + x) * 4;
			
			Vec4F addition = vec4F(
				(float)pixels[index] / 255.0f,
				(float)pixels[index + 1] / 255.0f,
				(float)pixels[index + 2] / 255.0f,
				(float)pixels[index + 3] / 255.0f);
			color = addVec4F(color, addition);
		}

		colors[x] = divValVec4F(color, (float)size.y);
	}

	GradSkyAmbient gradSkyAmbient = malloc(
		sizeof(struct GradSkyAmbient));

	if (gradSkyAmbient == NULL)
	{
		free(colors);
		return NULL;
	}

	gradSkyAmbient->colors = colors;
	gradSkyAmbient->count = size.x;
	return gradSkyAmbient;
}
void destroyGradSkyAmbient(
	GradSkyAmbient gradSkyAmbient)
{
	if (gradSkyAmbient == NULL)
		return;

	free(gradSkyAmbient->colors);
	free(gradSkyAmbient);
}
Vec4F getGradSkyAmbientColor(
	GradSkyAmbient gradSkyAmbient,
	float dayTime)
{
	assert(gradSkyAmbient != NULL);
	assert(dayTime >= 0.0f);
	assert(dayTime <= 1.0f);

	Vec4F* colors = gradSkyAmbient->colors;
	size_t colorCount = gradSkyAmbient->count;

	dayTime = (float)(colorCount - 1) * dayTime;

	float secondValue = dayTime - (float)((int)dayTime);
	float firstValue = 1.0f - secondValue;

	Vec4F firstColor = colors[(size_t)dayTime];
	Vec4F secondColor = colors[(size_t)dayTime + 1];

	return vec4F(
		firstColor.x * firstValue + secondColor.x * secondValue,
		firstColor.y * firstValue + secondColor.y * secondValue,
		firstColor.z * firstValue + secondColor.z * secondValue,
		firstColor.w * firstValue + secondColor.w * secondValue);
}

Sampler createGradSkySampler(Window window)
{
	assert(window != NULL);

	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		false,
		CLAMP_TO_EDGE_IMAGE_WRAP,
		CLAMP_TO_EDGE_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		NEVER_COMPARE_OPERATOR,
		false,
		defaultMipmapLodRange,
		DEFAULT_MIPMAP_LOD_BIAS);
}

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(Vec3F),
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[1] = {
	{
		0,
		0,
		VK_FORMAT_R32G32B32_SFLOAT,
		0,
	},
};
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Mat4F),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(Mat4F),
		sizeof(Vec4F) * 2,
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
inline static VkImageView createVkImageView(
	VkDevice device,
	VkImage image)
{
	VkImageViewCreateInfo imageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		image,
		VK_IMAGE_VIEW_TYPE_2D,
		VK_FORMAT_R8G8B8A8_SRGB,
		{
			0,
			0,
			0,
			0,
		},
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		},
	};

	VkImageView imageView;

	VkResult result = vkCreateImageView(
		device,
		&imageViewCreateInfo,
		NULL,
		&imageView);

	if(result != VK_SUCCESS)
		return NULL;

	return imageView;
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
		VkDescriptorImageInfo descriptorImageInfos[1] =
		{
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

static void onVkHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(window);
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
	PipelineHandle* handle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipeline->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;
	VkPipelineLayout layout = pipeline->vk.layout;

	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Mat4F),
		&handle->vk.mvp);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(Mat4F),
		sizeof(Vec4F),
		&handle->vk.sunDir);
	vkCmdPushConstants(
		commandBuffer,
		layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		sizeof(Mat4F) + sizeof(Vec4F),
		sizeof(Vec4F),
		&handle->vk.sunColor);
}
static void onVkHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipeline->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;

	vkCmdBindDescriptorSets(
		vkWindow->currenCommandBuffer,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		pipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSets[bufferIndex],
		0,
		NULL);
}
static void onVkHandleResize(
	Pipeline pipeline,
	void* createInfo)
{
	Window window = pipeline->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;
	PipelineHandle* handle = pipeline->vk.handle;

	if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		free(handle->vk.descriptorSets);

		vkDestroyDescriptorPool(
			device,
			handle->vk.descriptorPool,
			NULL);

		VkDescriptorPool descriptorPool = createVkDescriptorPool(
			device,
			bufferCount);

		if (descriptorPool == NULL)
			abort();

		VkDescriptorSet* descriptorSets = createVkDescriptorSets(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			handle->vk.sampler->vk.handle,
			handle->vk.imageView);

		if (descriptorSets == NULL)
			abort();

		handle->vk.descriptorPool = descriptorPool;
		handle->vk.descriptorSets = descriptorSets;
		handle->vk.bufferCount = bufferCount;
	}

	Vec2U framebufferSize =
		getWindowFramebufferSize(window);
	Vec4I size = vec4I(0, 0,
		(int32_t)framebufferSize.x,
		(int32_t)framebufferSize.y);
	pipeline->vk.state.viewport = size;
	pipeline->vk.state.scissor = size;

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		1,
		&handle->vk.descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
}
inline static Pipeline createVkHandle(
	Window window,
	Shader* shaders,
	uint8_t shaderCount,
	VkSampler sampler,
	VkImage image,
	const PipelineState* state,
	PipelineHandle* handle)
{
	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
		return NULL;

	VkPipelineCreateInfo createInfo = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		1,
		&descriptorSetLayout,
		2,
		pushConstantRanges,
	};

	Pipeline pipeline = createPipeline(
		window,
		GRAD_SKY_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onVkHandleDestroy,
		NULL,
		onVkUniformsSet,
		onVkHandleResize,
		handle,
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
		image);

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

	handle->vk.descriptorSetLayout = descriptorSetLayout;
	handle->vk.descriptorPool = descriptorPool;
	handle->vk.imageView = imageView;
	handle->vk.descriptorSets = descriptorSets;
	handle->vk.bufferCount = bufferCount;
	return pipeline;
}
#endif

static void onGlHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
}
static void onGlHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;

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
static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.mvp);
	glUniform4fv(
		handle->gl.sunDirLocation,
		1,
		(const GLfloat*)&handle->gl.sunDir);
	glUniform4fv(
		handle->gl.sunColorLocation,
		1,
		(const GLfloat*)&handle->gl.sunColor);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F),
		0);

	assertOpenGL();
}
static void onGlHandleResize(
	Pipeline pipeline,
	void* createInfo)
{
	Vec2U framebufferSize = getWindowFramebufferSize(
		pipeline->gl.window);
	Vec4I size = vec4I(0, 0,
		(int32_t)framebufferSize.x,
		(int32_t)framebufferSize.y);
	pipeline->gl.state.viewport = size;
	pipeline->gl.state.scissor = size;
}
inline static Pipeline createGlHandle(
	Window window,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* handle)
{
	Pipeline pipeline = createPipeline(
		window,
		GRAD_SKY_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onGlHandleDestroy,
		onGlHandleBind,
		onGlUniformsSet,
		onGlHandleResize,
		handle,
		NULL);

	if (pipeline == NULL)
		return NULL;

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation, sunDirLocation,
		sunColorLocation, textureLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_SunDir",
		&sunDirLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_SunColor",
		&sunColorLocation);
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

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.sunDirLocation = sunDirLocation;
	handle->gl.sunColorLocation = sunColorLocation;
	handle->gl.textureLocation = textureLocation;
	return pipeline;
}

Pipeline createExtGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const PipelineState* state)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(texture != NULL);
	assert(sampler != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);
	assert(getImageWindow(texture) == window);
	assert(getSamplerWindow(sampler) == window);

	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GraphicsAPI api = getWindowGraphicsAPI(window);

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		pipeline = createVkHandle(
			window,
			shaders,
			2,
			sampler->vk.handle,
			texture->vk.handle,
			state,
			handle);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipeline = createGlHandle(
			window,
			shaders,
			2,
			state,
			handle);
	}
	else
	{
		abort();
	}

	if (pipeline == NULL)
	{
		free(handle);
		return NULL;
	}

	handle->vk.texture = texture;
	handle->vk.sampler = sampler;
	handle->vk.mvp = identMat4F();
	handle->vk.mvp = identMat4F();
	handle->vk.sunDir = zeroVec4F();
	handle->vk.sunColor = oneVec4F();
	return pipeline;
}
Pipeline createGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	assert(window != NULL);

	Vec2U framebufferSize =
		getWindowFramebufferSize(window);
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
		true,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		defaultDepthRange,
		size,
	};

	return createExtGradSkyPipeline(
		window,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state);
}

Image getGradSkyPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.texture;
}
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.sampler;
}

Mat4F getGradSkyPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setGradSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

Vec3F getGradSkyPipelineSunDir(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	Vec4F sunDir =
		pipelineHandle->vk.sunDir;
	return vec3F(
		sunDir.x,
		sunDir.y,
		sunDir.z);
}
void setGradSkyPipelineSunDir(
	Pipeline pipeline,
	Vec3F sunDir)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.sunDir = vec4F(
		sunDir.x,
		sunDir.y,
		sunDir.z,
		0.0f);
}

Vec4F getGradSkyPipelineSunColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.sunColor;
}
void setGradSkyPipelineSunColor(
	Pipeline pipeline,
	Vec4F sunColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.sunColor = sunColor;
}
