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

#include "mpgx/pipelines/gradient_sky_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

struct GradientSkyAmbient_T
{
	LinearColor* colors;
	size_t count;
};

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	Vec4F sunDir;
	LinearColor sunColor;
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
	GLint sunDirLocation;
	GLint sunColorLocation;
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

MpgxResult createGradientSkyAmbient(
	ImageData gradient,
	GradientSkyAmbient* gradientSkyAmbient)
{
	assert(gradient != NULL);
	assert(gradientSkyAmbient != NULL);
	assert(getImageDataChannelCount(gradient) == 4);

	Vec2U size = getImageDataSize(gradient);

	LinearColor* colors = malloc(
		sizeof(Vec4F) * size.x);

	if (colors == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	const uint8_t* pixels = getImageDataPixels(gradient);

	for (uint32_t x = 0; x < size.x; x++)
	{
		LinearColor color = zeroLinearColor;

		for (uint32_t y = 0; y < size.y; y++)
		{
			size_t index = (y * size.x + x) * 4;
			
			LinearColor addition = srgbToLinearColor(srgbColor(
				pixels[index],
				pixels[index + 1],
				pixels[index + 2],
				pixels[index + 3]));
			color = addLinearColor(color, addition);
		}

		colors[x] = divValLinearColor(color, (float)size.y);
	}

	GradientSkyAmbient gradientSkyAmbientInstance = malloc(
		sizeof(GradientSkyAmbient_T));

	if (gradientSkyAmbient == NULL)
	{
		free(colors);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	gradientSkyAmbientInstance->colors = colors;
	gradientSkyAmbientInstance->count = size.x;

	*gradientSkyAmbient = gradientSkyAmbientInstance;
	return SUCCESS_MPGX_RESULT;
}
void destroyGradientSkyAmbient(
	GradientSkyAmbient gradientSkyAmbient)
{
	if (gradientSkyAmbient == NULL)
		return;

	free(gradientSkyAmbient->colors);
	free(gradientSkyAmbient);
}
LinearColor getGradientSkyAmbientColor(
	GradientSkyAmbient gradientSkyAmbient,
	float dayTime)
{
	assert(gradientSkyAmbient != NULL);
	assert(dayTime >= 0.0f);
	assert(dayTime <= 1.0f);

	LinearColor* colors = gradientSkyAmbient->colors;
	size_t colorCount = gradientSkyAmbient->count;

	dayTime = (float)(colorCount - 1) * dayTime;

	float secondValue = dayTime - (float)((int)dayTime);
	float firstValue = 1.0f - secondValue;

	LinearColor firstColor = colors[(size_t)dayTime];
	LinearColor secondColor = colors[(size_t)dayTime + 1];

	return linearColor(
		firstColor.r * firstValue + secondColor.r * secondValue,
		firstColor.g * firstValue + secondColor.g * secondValue,
		firstColor.b * firstValue + secondColor.b * secondValue,
		firstColor.a * firstValue + secondColor.a * secondValue);
}

MpgxResult createGradientSkySampler(
	Window window,
	Sampler* gradientSkySampler)
{
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
		DEFAULT_MIPMAP_LOD_BIAS,
		gradientSkySampler);
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
static MpgxResult onVkResize(
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
		1,
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
	Window window = framebuffer->vk.window;
	VkWindow vkWindow = getVkWindow(window);
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
		1,
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
		GRADIENT_SKY_PIPELINE_NAME,
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
	glUniform4fv(
		handle->gl.sunDirLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.sunDir);
	glUniform4fv(
		handle->gl.sunColorLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.sunColor);

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
static MpgxResult onGlResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
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
	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		GRADIENT_SKY_PIPELINE_NAME,
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
		destroyGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.sunDirLocation = sunDirLocation;
	handle->gl.sunColorLocation = sunColorLocation;
	handle->gl.textureLocation = textureLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createGradientSkyPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	const GraphicsPipelineState* state,
	GraphicsPipeline* gradientSkyPipeline)
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

	Handle handle = calloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.texture = texture;
	handle->base.sampler = sampler;
	handle->base.vpc.mvp = identMat4F;
	handle->base.fpc.sunDir = zeroVec4F;
	handle->base.fpc.sunColor = whiteLinearColor;

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
			gradientSkyPipeline);
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
			gradientSkyPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
MpgxResult createGradientSkyPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	GraphicsPipeline* graphicsPipeline)
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

	return createGradientSkyPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state,
		graphicsPipeline);
}

Image getGradientSkyPipelineTexture(
	GraphicsPipeline gradientSkyPipeline)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	return handle->base.texture;
}
Sampler getGradientSkyPipelineSampler(
	GraphicsPipeline gradientSkyPipeline)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	return handle->base.sampler;
}

Mat4F getGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setGradientSkyPipelineMvp(
	GraphicsPipeline gradientSkyPipeline,
	Mat4F mvp)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

Vec3F getGradientSkyPipelineSunDir(
	GraphicsPipeline gradientSkyPipeline)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	Vec4F sunDir = handle->base.fpc.sunDir;
	return vec3F(
		sunDir.x,
		sunDir.y,
		sunDir.z);
}
void setGradientSkyPipelineSunDir(
	GraphicsPipeline gradientSkyPipeline,
	Vec3F sunDir)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	handle->base.fpc.sunDir = vec4F(
		sunDir.x,
		sunDir.y,
		sunDir.z,
		0.0f);
}

LinearColor getGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	return handle->base.fpc.sunColor;
}
void setGradientSkyPipelineSunColor(
	GraphicsPipeline gradientSkyPipeline,
	LinearColor sunColor)
{
	assert(gradientSkyPipeline != NULL);
	assert(strcmp(gradientSkyPipeline->base.name,
		GRADIENT_SKY_PIPELINE_NAME) == 0);
	Handle handle = gradientSkyPipeline->base.handle;
	handle->base.fpc.sunColor = sunColor;
}
