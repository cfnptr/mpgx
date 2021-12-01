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

#include "mpgx/pipelines/sprite_pipeline.h"
#include "mpgx/_source/pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	LinearColor color;
} FragmentPushConstants;
typedef struct BasePipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint colorLocation;
} GlPipelineHandle;
union PipelineHandle
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
};

typedef union PipelineHandle* PipelineHandle;

#if MPGX_SUPPORT_VULKAN
static const VkVertexInputBindingDescription vertexInputBindingDescriptions[1] = {
	{
		0,
		sizeof(Vec2F),
		VK_VERTEX_INPUT_RATE_VERTEX,
	},
};
static const VkVertexInputAttributeDescription vertexInputAttributeDescriptions[1] = {
	{
		0,
		0,
		VK_FORMAT_R32G32_SFLOAT,
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

static void onVkUniformsSet(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->vk.handle;
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
static bool onVkHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
	pipeline->vk.state.viewport = vec4I(0, 0,
		(int32_t)newSize.x,
		(int32_t)newSize.y);

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
	return true;
}
static void onVkHandleDestroy(void* handle)
{
	free((PipelineHandle)handle);
}
inline static Pipeline createVkHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
{
	VkPipelineCreateInfo createInfo = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	return createPipeline(
		framebuffer,
		SPRITE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		NULL,
		onVkUniformsSet,
		onVkHandleResize,
		onVkHandleDestroy,
		pipelineHandle,
		&createInfo);
}
#endif

static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle pipelineHandle = pipeline->gl.handle;

	glUniformMatrix4fv(
		pipelineHandle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.vpc.mvp);
	glUniform4fv(
		pipelineHandle->gl.colorLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.fpc.color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F),
		0);

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
static void onGlHandleDestroy(void* handle)
{
	free((PipelineHandle)handle);
}
inline static Pipeline createGlHandle(
	Framebuffer framebuffer,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle pipelineHandle)
{
	Pipeline pipeline = createPipeline(
		framebuffer,
		SPRITE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		NULL,
		onGlUniformsSet,
		onGlHandleResize,
		onGlHandleDestroy,
		pipelineHandle,
		NULL);

	if (pipeline == NULL)
		return NULL;

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation, colorLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);
	result &= getGlUniformLocation(
		glHandle,
		"u_Color",
		&colorLocation);

	if (result == false)
	{
		destroyPipeline(pipeline, false);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.colorLocation = colorLocation;
	return pipeline;
}

Pipeline createExtSpritePipeline(
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
		sizeof(union PipelineHandle));

	if (pipelineHandle == NULL)
		return NULL;

	Window window = framebuffer->base.window;
	pipelineHandle->base.window = window;
	pipelineHandle->base.vpc.mvp = identMat4F;
	pipelineHandle->base.fpc.color = whiteLinearColor;

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
Pipeline createSpritePipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader)
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

	return createExtSpritePipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getSpritePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.mvp;
}
void setSpritePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.mvp = mvp;
}

LinearColor getSpritePipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.fpc.color;
}
void setSpritePipelineColor(
	Pipeline pipeline,
	LinearColor color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.fpc.color = color;
}
