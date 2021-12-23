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
#include "mpgx/_source/pipeline.h"

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
typedef struct BasePipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	Buffer* uniformBuffers;
	VkDescriptorSet* descriptorSets;
	uint32_t bufferCount;
#endif
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	UniformBuffer ub;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
} GlPipelineHandle;
union PipelineHandle_T
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
};

typedef union PipelineHandle_T PipelineHandle_T;
typedef PipelineHandle_T* PipelineHandle;

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

static void onVkHandleBind(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	uint32_t bufferIndex = vkWindow->bufferIndex;
	Buffer buffer = pipelineHandle->vk.uniformBuffers[bufferIndex];

	setVkBufferData(
		vkWindow->allocator,
		buffer->vk.allocation,
		&pipelineHandle->vk.ub,
		sizeof(UniformBuffer),
		0);
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
static void onVkUniformsSet(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);

	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		pipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
		&pipelineHandle->vk.vpc);
}
static bool onVkHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	PipelineHandle pipelineHandle = pipeline->vk.handle;
	Window window = pipelineHandle->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	uint32_t bufferCount = vkWindow->swapchain->bufferCount;

	if (bufferCount != pipelineHandle->vk.bufferCount)
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
			pipelineHandle->vk.descriptorSetLayout,
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

		free(pipelineHandle->vk.descriptorSets);

		destroyVkUniformBuffers(
			pipelineHandle->vk.bufferCount,
			pipelineHandle->vk.uniformBuffers);
		vkDestroyDescriptorPool(
			device,
			pipelineHandle->vk.descriptorPool,
			NULL);

		pipelineHandle->vk.descriptorPool = descriptorPool;
		pipelineHandle->vk.uniformBuffers = uniformBuffers;
		pipelineHandle->vk.descriptorSets = descriptorSets;
		pipelineHandle->vk.bufferCount = bufferCount;
	}

	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = pipeline->vk.state.viewport.z +
		pipeline->vk.state.viewport.w == 0;
	if (dynamic == false)
		pipeline->vk.state.viewport = size;

	dynamic = pipeline->vk.state.scissor.z +
		pipeline->vk.state.scissor.w == 0;
	if (dynamic == false)
		pipeline->vk.state.scissor = size;

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		1,
		&pipelineHandle->vk.descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
	return true;
}
static void onVkHandleDestroy(void* handle)
{
	PipelineHandle pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkDevice device = vkWindow->device;

	free(pipelineHandle->vk.descriptorSets);
	destroyVkUniformBuffers(
		pipelineHandle->vk.bufferCount,
		pipelineHandle->vk.uniformBuffers);
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
inline static Pipeline createVkHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
{
	Window window = framebuffer->vk.window;
	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
	{
		free(pipelineHandle);
		return NULL;
	}

	VkPipelineCreateInfo createInfo = {
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
		free(pipelineHandle);
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
		free(pipelineHandle);
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
		free(pipelineHandle);
		return NULL;
	}

	pipelineHandle->vk.descriptorSetLayout = descriptorSetLayout;
	pipelineHandle->vk.descriptorPool = descriptorPool;
	pipelineHandle->vk.uniformBuffers = uniformBuffers;
	pipelineHandle->vk.descriptorSets = descriptorSets;
	pipelineHandle->vk.bufferCount = bufferCount;

	return createPipeline(
		framebuffer,
		DIFFUSE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onVkHandleBind,
		onVkUniformsSet,
		onVkHandleResize,
		onVkHandleDestroy,
		pipelineHandle,
		&createInfo);
}
#endif

static void onGlHandleBind(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->gl.handle;
	Buffer uniformBuffer = pipelineHandle->gl.uniformBuffer;

	setGlBufferData(
		uniformBuffer->gl.glType,
		uniformBuffer->gl.handle,
		&pipelineHandle->gl.ub,
		sizeof(UniformBuffer),
		0);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		uniformBuffer->gl.handle);
	assertOpenGL();
}
static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->gl.handle;

	glUniformMatrix4fv(
		pipelineHandle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.vpc.mvp);
	glUniformMatrix4fv(
		pipelineHandle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.vpc.normal);

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
static bool onGlHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	Vec4U size = vec4U(0, 0,
		newSize.x, newSize.y);

	bool dynamic = pipeline->vk.state.viewport.z +
		pipeline->vk.state.viewport.w == 0;
	if (dynamic == false)
		pipeline->vk.state.viewport = size;

	dynamic = pipeline->vk.state.scissor.z +
		pipeline->vk.state.scissor.w == 0;
	if (dynamic == false)
		pipeline->vk.state.scissor = size;
	return true;
}
static void onGlHandleDestroy(void* handle)
{
	PipelineHandle pipelineHandle = handle;
	destroyBuffer(pipelineHandle->gl.uniformBuffer);
	free(pipelineHandle);
}
inline static Pipeline createGlHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
{
	Buffer uniformBuffer = createBuffer(
		framebuffer->gl.window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(UniformBuffer),
		false);

	if (uniformBuffer == NULL)
	{
		free(pipelineHandle);
		return NULL;
	}

	pipelineHandle->gl.uniformBuffer = uniformBuffer;

	Pipeline pipeline = createPipeline(
		framebuffer,
		DIFFUSE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onGlHandleBind,
		onGlUniformsSet,
		onGlHandleResize,
		onGlHandleDestroy,
		pipelineHandle,
		NULL);

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
		destroyPipeline(pipeline, false);
		return NULL;
	}

	glUniformBlockBinding(
		glHandle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.normalLocation = normalLocation;
	return pipeline;
}

Pipeline createExtDiffusePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const PipelineState* state)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);

	PipelineHandle pipelineHandle = malloc(
		sizeof(PipelineHandle_T));

	if (pipelineHandle == NULL)
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
	pipelineHandle->base.window = window;
	pipelineHandle->base.vpc.mvp = identMat4F;
	pipelineHandle->base.vpc.normal = identMat4F;
	pipelineHandle->base.ub = ub;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkHandle(
			framebuffer,
			shaders,
			2,
			state,
			pipelineHandle);
#else
		abort();
#endif
	}
	else if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		return createGlHandle(
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
}
Pipeline createDiffusePipeline(
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

	PipelineState state = {
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

	return createExtDiffusePipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getDiffusePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.mvp;
}
void setDiffusePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.mvp = mvp;
}

Mat4F getDiffusePipelineNormal(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.normal;
}
void setDiffusePipelineNormal(
	Pipeline pipeline,
	Mat4F normal)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.normal = normal;
}

LinearColor getDiffusePipelineObjectColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.ub.objectColor;
}
void setDiffusePipelineObjectColor(
	Pipeline pipeline,
	LinearColor objectColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.ub.objectColor = objectColor;
}

LinearColor getDiffusePipelineAmbientColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.ub.ambientColor;
}
void setDiffusePipelineAmbientColor(
	Pipeline pipeline,
	LinearColor ambientColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.ub.ambientColor = ambientColor;
}

LinearColor getDiffusePipelineLightColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.ub.lightColor;
}
void setDiffusePipelineLightColor(
	Pipeline pipeline,
	LinearColor lightColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.ub.lightColor = lightColor;
}

Vec3F getDiffusePipelineLightDirection(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	Vec4F lightDirection =
		pipelineHandle->base.ub.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	Pipeline pipeline,
	Vec3F lightDirection)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	lightDirection = normVec3F(lightDirection);
	pipelineHandle->base.ub.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
