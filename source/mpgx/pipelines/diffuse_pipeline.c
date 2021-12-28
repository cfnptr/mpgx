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

#include "mpgx/pipelines/diffuse_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
	Mat4F normal;
} VertexPushConstants;
typedef struct UniformBuffer
{
	LinearColor objectColor;
	LinearColor ambientColor;
	LinearColor lightColor;
	Vec4F lightDirection;
} UniformBuffer;
typedef struct BaseHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	Buffer* uniformBuffers;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
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
		sizeof(Vec3F) * 2,
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
		VK_FORMAT_R32G32B32_SFLOAT,
		sizeof(Vec3F),
	},
};
static const VkPushConstantRange pushConstantRanges[1] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
};

inline static VkDescriptorSetLayout createVkDescriptorSetLayout(
	VkDevice device)
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[1] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
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
			VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
			bufferCount,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfos[1] = {
		{
			VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
			NULL,
			0,
			bufferCount,
			1,
			descriptorPoolSizes,
		},
	};

	VkDescriptorPool descriptorPool;

	VkResult result = vkCreateDescriptorPool(
		device,
		descriptorPoolCreateInfos,
		NULL,
		&descriptorPool);

	if (result != VK_SUCCESS)
		return NULL;

	return descriptorPool;
}
inline static void destroyVkUniformBuffers(
	uint32_t bufferCount,
	Buffer* uniformBuffers)
{
	for (uint32_t i = 0; i < bufferCount; i++)
		destroyBuffer(uniformBuffers[i]);

	free(uniformBuffers);
}
inline static Buffer* createVkUniformBuffers(
	Window window,
	uint32_t bufferCount)
{
	Buffer* buffers = malloc(
		bufferCount * sizeof(Buffer));

	if (buffers == NULL)
		return NULL;

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		Buffer buffer = createBuffer(
			window,
			UNIFORM_BUFFER_TYPE,
			NULL,
			sizeof(UniformBuffer),
			false);

		if (buffer == NULL)
		{
			destroyVkUniformBuffers(i, buffers);
			return NULL;
		}

		buffers[i] = buffer;
	}

	return buffers;
}
inline static VkDescriptorSet* createVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t bufferCount,
	Buffer* uniformBuffers)
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
		VkDescriptorBufferInfo descriptorBufferInfos[1] = {
			{
				uniformBuffers[i]->vk.handle,
				0,
				sizeof(UniformBuffer),
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
				VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
				NULL,
				descriptorBufferInfos,
				NULL,
			}
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
	Buffer buffer = handle->vk.uniformBuffers[bufferIndex];

	setVkBufferData(
		vkWindow->allocator,
		buffer->vk.allocation,
		&handle->vk.ub,
		sizeof(UniformBuffer),
		0);
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

	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		graphicsPipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&handle->vk.vpc);
}
static bool onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
	Handle handle = graphicsPipeline->vk.handle;
	Window window = handle->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != handle->vk.bufferCount)
	{
		VkDevice device = vkWindow->device;

		VkDescriptorPool descriptorPool = createVkDescriptorPool(
			device,
			bufferCount);

		if (descriptorPool == NULL)
			return false;

		Buffer* uniformBuffers = createVkUniformBuffers(
			window,
			bufferCount);

		if (uniformBuffers == NULL)
		{
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			return false;
		}

		VkDescriptorSet* descriptorSets = createVkDescriptorSets(
			device,
			handle->vk.descriptorSetLayout,
			descriptorPool,
			bufferCount,
			uniformBuffers);

		if (descriptorSets == NULL)
		{
			destroyVkUniformBuffers(
				bufferCount,
				uniformBuffers);
			vkDestroyDescriptorPool(
				device,
				descriptorPool,
				NULL);
			return false;
		}

		free(handle->vk.descriptorSets);

		destroyVkUniformBuffers(
			handle->vk.bufferCount,
			handle->vk.uniformBuffers);
		vkDestroyDescriptorPool(
			device,
			handle->vk.descriptorPool,
			NULL);

		handle->vk.descriptorPool = descriptorPool;
		handle->vk.uniformBuffers = uniformBuffers;
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
	return true;
}
static void onVkDestroy(void* _handle)
{
	Handle handle = _handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkDevice device = vkWindow->device;

	free(handle->vk.descriptorSets);
	destroyVkUniformBuffers(
		handle->vk.bufferCount,
		handle->vk.uniformBuffers);
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
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount)
{
	Window window = framebuffer->vk.window;
	VkWindow vkWindow = getVkWindow(window);
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
		1,
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

	Buffer* uniformBuffers = createVkUniformBuffers(
		window,
		bufferCount);

	if (uniformBuffers == NULL)
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

	VkDescriptorSet* descriptorSets = createVkDescriptorSets(
		device,
		descriptorSetLayout,
		descriptorPool,
		bufferCount,
		uniformBuffers);

	if (descriptorSets == NULL)
	{
		destroyVkUniformBuffers(
			bufferCount,
			uniformBuffers);
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
	handle->vk.uniformBuffers = uniformBuffers;
	handle->vk.descriptorSets = descriptorSets;
	handle->vk.bufferCount = bufferCount;

	return createGraphicsPipeline(
		framebuffer,
		DIFFUSE_PIPELINE_NAME,
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
	Buffer uniformBuffer = handle->gl.uniformBuffer;

	setGlBufferData(
		uniformBuffer->gl.glType,
		uniformBuffer->gl.handle,
		&handle->gl.ub,
		sizeof(UniformBuffer),
		0);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		uniformBuffer->gl.handle);
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
	glUniformMatrix4fv(
		handle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
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
static void onGlDestroy(void* _handle)
{
	Handle handle = _handle;
	destroyBuffer(handle->gl.uniformBuffer);
	free(handle);
}
inline static GraphicsPipeline createGlPipeline(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const GraphicsPipelineState* state,
	Handle handle)
{
	Buffer uniformBuffer = createBuffer(
		framebuffer->gl.window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(UniformBuffer),
		false);

	if (uniformBuffer == NULL)
	{
		free(handle);
		return NULL;
	}

	handle->gl.uniformBuffer = uniformBuffer;

	GraphicsPipeline pipeline = createGraphicsPipeline(
		framebuffer,
		DIFFUSE_PIPELINE_NAME,
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

	GLint mvpLocation, normalLocation;
	GLuint uniformBlockIndex;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Normal",
		&normalLocation);
	result &= getGlUniformBlockIndex(
		glHandle,
		"UniformBuffer",
		&uniformBlockIndex);

	if (result == false)
	{
		destroyGraphicsPipeline(pipeline, false);
		return NULL;
	}

	glUniformBlockBinding(
		glHandle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.normalLocation = normalLocation;
	return pipeline;
}
#endif

GraphicsPipeline createDiffusePipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);

	Handle handle = malloc(sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, -3.0f, 6.0f));

	UniformBuffer ub = {
		whiteLinearColor,
		valueLinearColor(0.5f),
		whiteLinearColor,
		vec4F(
			lightDirection.x,
			lightDirection.y,
			lightDirection.z,
			0.0f),
	};

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.vpc.mvp = identMat4F;
	handle->base.vpc.normal = identMat4F;
	handle->base.ub = ub;

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
			shaders,
			2,
			state,
			handle);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
GraphicsPipeline createDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader)
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

	return createDiffusePipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setDiffusePipelineMvp(
	GraphicsPipeline diffusePipeline,
	Mat4F mvp)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

Mat4F getDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.vpc.normal;
}
void setDiffusePipelineNormal(
	GraphicsPipeline diffusePipeline,
	Mat4F normal)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.vpc.normal = normal;
}

LinearColor getDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.objectColor;
}
void setDiffusePipelineObjectColor(
	GraphicsPipeline diffusePipeline,
	LinearColor objectColor)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.objectColor = objectColor;
}

LinearColor getDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.ambientColor;
}
void setDiffusePipelineAmbientColor(
	GraphicsPipeline diffusePipeline,
	LinearColor ambientColor)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.ambientColor = ambientColor;
}

LinearColor getDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	return handle->base.ub.lightColor;
}
void setDiffusePipelineLightColor(
	GraphicsPipeline diffusePipeline,
	LinearColor lightColor)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	handle->base.ub.lightColor = lightColor;
}

Vec3F getDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	Vec4F lightDirection = handle->base.ub.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	GraphicsPipeline diffusePipeline,
	Vec3F lightDirection)
{
	assert(diffusePipeline != NULL);
	assert(strcmp(diffusePipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	Handle handle = diffusePipeline->base.handle;
	lightDirection = normVec3F(lightDirection);
	handle->base.ub.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
