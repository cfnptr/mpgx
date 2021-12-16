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

#include "mpgx/pipelines/simpshad_pipeline.h"
#include "mpgx/_source/pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct BasePipelineHandle
{
	Window window;
	VertexPushConstants vpc;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Window window;
	VertexPushConstants vpc;
	GLint mvpLocation;
} GlPipelineHandle;
union PipelineHandle_T
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
	GlPipelineHandle gl;
};

typedef union PipelineHandle_T PipelineHandle_T;
typedef PipelineHandle_T* PipelineHandle;

Sampler createSimpShadSampler(Window window)
{
	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		false,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		LESS_COMPARE_OPERATOR,
		true,
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
static const VkPushConstantRange pushConstantRanges[1] = {
	{
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(VertexPushConstants),
	},
};

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
	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		1,
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
		1,
		pushConstantRanges,
	};

	return createPipeline(
		framebuffer,
		SIMPSHAD_PIPELINE_NAME,
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
static bool onGlHandleResize(
	Pipeline pipeline,
	Vec2U newSize,
	void* createInfo)
{
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
		SIMPSHAD_PIPELINE_NAME,
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

	GLint mvpLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);

	if (result == false)
	{
		destroyPipeline(pipeline, false);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.mvpLocation = mvpLocation;
	return pipeline;
}

Pipeline createExtSimpShadPipeline(
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

	Window window = framebuffer->base.window;
	pipelineHandle->base.window = window;
	pipelineHandle->base.vpc.mvp = identMat4F;

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
Pipeline createSimpShadPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	uint32_t shadowMapLength)
{
	assert(framebuffer != NULL);
	assert(shadowMapLength != 0);

	Vec4U size = vec4U(0, 0,
		shadowMapLength,
		shadowMapLength);
	Vec2F depthBias = vec2F(
		1.1f,4.0f);

	PipelineState state = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATOR,
		NONE_COLOR_COMPONENT,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ZERO_BLEND_FACTOR,
		ADD_BLEND_OPERATOR,
		ADD_BLEND_OPERATOR,
		false,
		false,
		true,
		true,
		false,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		size,
		size,
		defaultDepthRange,
		depthBias,
		defaultBlendColor,
	};

	return createExtSimpShadPipeline(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getSimpShadPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SIMPSHAD_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	return pipelineHandle->base.vpc.mvp;
}
void setSimpShadPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		pipeline->base.name,
		SIMPSHAD_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		pipeline->base.handle;
	pipelineHandle->base.vpc.mvp = mvp;
}
