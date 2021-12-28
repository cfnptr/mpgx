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

#include "mpgx/pipelines/color_pipeline.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/graphics_pipeline.h"

#include <string.h>

typedef struct VertexPushConstants
{
	Mat4F mvp;
} VertexPushConstants;
typedef struct FragmentPushConstants
{
	LinearColor color;
} FragmentPushConstants;
typedef struct BaseHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
} VkHandle;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlHandle
{
	Window window;
	VertexPushConstants vpc;
	FragmentPushConstants fpc;
	GLint mvpLocation;
	GLint colorLocation;
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
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	*(VkGraphicsPipelineCreateData*)createData = _createData;
	return true;
}
static void onVkDestroy(void* handle)
{
	free((Handle)handle);
}
inline static GraphicsPipeline createVkPipeline(
	Framebuffer framebuffer,
	const GraphicsPipelineState* state,
	Handle handle,
	Shader* shaders,
	uint8_t shaderCount)
{
	VkGraphicsPipelineCreateData createData = {
		1,
		vertexInputBindingDescriptions,
		1,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		2,
		pushConstantRanges,
	};

	return createGraphicsPipeline(
		framebuffer,
		COLOR_PIPELINE_NAME,
		state,
		NULL,
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
static void onGlUniformsSet(GraphicsPipeline graphicsPipeline)
{
	Handle handle = graphicsPipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.vpc.mvp);
	glUniform4fv(
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.fpc.color);

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
		COLOR_PIPELINE_NAME,
		state,
		NULL,
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
		destroyGraphicsPipeline(pipeline, false);
		return NULL;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.colorLocation = colorLocation;
	return pipeline;
}
#endif

GraphicsPipeline createColorPipelineExt(
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

	Window window = framebuffer->base.window;
	handle->base.window = window;
	handle->base.vpc.mvp = identMat4F;
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
	}
}
GraphicsPipeline createColorPipeline(
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

	return createColorPipelineExt(
		framebuffer,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getColorPipelineMvp(
	GraphicsPipeline colorPipeline)
{
	assert(colorPipeline != NULL);
	assert(strcmp(colorPipeline->base.name,
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = colorPipeline->base.handle;
	return handle->base.vpc.mvp;
}
void setColorPipelineMvp(
	GraphicsPipeline colorPipeline,
	Mat4F mvp)
{
	assert(colorPipeline != NULL);
	assert(strcmp(colorPipeline->base.name,
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = colorPipeline->base.handle;
	handle->base.vpc.mvp = mvp;
}

LinearColor getColorPipelineColor(
	GraphicsPipeline colorPipeline)
{
	assert(colorPipeline != NULL);
	assert(strcmp(colorPipeline->base.name,
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = colorPipeline->base.handle;
	return handle->base.fpc.color;
}
void setColorPipelineColor(
	GraphicsPipeline colorPipeline,
	LinearColor color)
{
	assert(colorPipeline != NULL);
	assert(strcmp(colorPipeline->base.name,
		COLOR_PIPELINE_NAME) == 0);
	Handle handle = colorPipeline->base.handle;
	handle->base.fpc.color = color;
}
