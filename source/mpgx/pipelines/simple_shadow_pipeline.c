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

#include "mpgx/pipelines/simple_shadow_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct BaseHandle
{
	Window window;
	VertexPushConstants vpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	VertexPushConstants vpc;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Window window;
	VertexPushConstants vpc;
	GLint mvpLocation;
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

MpgxResult createSimpleShadowSampler(
	Window window,
	Sampler* simpleShadowSampler)
{
	assert(window != NULL);
	assert(simpleShadowSampler != NULL);

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
		DEFAULT_MIPMAP_LOD_BIAS,
		simpleShadowSampler);
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

static void onVkUniformsSet(GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

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
static MpgxResult onVkResize(
	GraphicsPipeline graphicsPipeline,
	Vec2U newSize,
	void* createData)
{
	assert(graphicsPipeline != NULL);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData != NULL);

	VkGraphicsPipelineCreateData _createData = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		1,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
	return SUCCESS_MPGX_RESULT;
}
static void onVkDestroy(void* handle)
{
	free((Handle)handle);
}
inline static MpgxResult createVkPipeline(
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

	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		1,
		pushConstantRanges,
	};

	GraphicsPipeline graphicsPipelineInstance;

	MpgxResult mpgxResult = createGraphicsPipeline(
		framebuffer,
		SIMPLE_SHADOW_PIPELINE_NAME,
		state,
		NULL,
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
static void onGlUniformsSet(
	GraphicsPipeline graphicsPipeline)
{
	assert(graphicsPipeline != NULL);

	Handle handle = graphicsPipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.mvp);

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
	assert(graphicsPipeline != NULL);
	assert(newSize.x > 0);
	assert(newSize.y > 0);
	assert(createData == NULL);
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
		SIMPLE_SHADOW_PIPELINE_NAME,
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

	GLint mvpLocation;

	bool result = getGlUniformLocation(
		glHandle,
		"u_MVP",
		&mvpLocation);

	if (result == false)
	{
		destroyGraphicsPipeline(
			graphicsPipelineInstance,
			false);
		return BAD_SHADER_CODE_MPGX_RESULT;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;

	*graphicsPipeline = graphicsPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

MpgxResult createSimpleShadowPipelineExt(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	const GraphicsPipelineState* state,
	GraphicsPipeline* simpleShadowPipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(state != NULL);
	assert(simpleShadowPipeline != NULL);
	assert(vertexShader->base.type == VERTEX_SHADER_TYPE);
	assert(fragmentShader->base.type == FRAGMENT_SHADER_TYPE);
	assert(vertexShader->base.window == framebuffer->base.window);
	assert(fragmentShader->base.window == framebuffer->base.window);

	Handle handle = calloc(1, sizeof(Handle_T));

	if (handle == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.vpc.mvp = identMat4F;

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
			2,
			simpleShadowPipeline);
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
			simpleShadowPipeline);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}
MpgxResult createSimpleShadowPipeline(
	Framebuffer framebuffer,
	Shader vertexShader,
	Shader fragmentShader,
	uint32_t shadowMapLength,
	GraphicsPipeline* simpleShadowPipeline)
{
	assert(framebuffer != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(shadowMapLength != 0);
	assert(simpleShadowPipeline != NULL);

	Vec4U size = vec4U(0, 0,
		shadowMapLength,
		shadowMapLength);
	Vec2F depthBias = vec2F(
		1.1f,4.0f);

	GraphicsPipelineState state = {
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

	return createSimpleShadowPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state,
		simpleShadowPipeline);
}

Mat4F getSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline)
{
	assert(simpleShadowPipeline != NULL);
	assert(strcmp(simpleShadowPipeline->base.name,
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);
	Handle handle = simpleShadowPipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setSimpleShadowPipelineMvp(
	GraphicsPipeline simpleShadowPipeline,
	Mat4F mvp)
{
	assert(simpleShadowPipeline != NULL);
	assert(strcmp(simpleShadowPipeline->base.name,
		SIMPLE_SHADOW_PIPELINE_NAME) == 0);
	Handle handle = simpleShadowPipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}
