#include "mpgx/pipelines/sprite_pipeline.h"
#include "mpgx/_source/pipeline.h"

#include <string.h>

typedef struct VkPipelineHandle
{
	Mat4F mvp;
	Vec4F color;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Mat4F mvp;
	Vec4F color;
	GLint mvpLocation;
	GLint colorLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

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
		sizeof(Mat4F),
	},
	{
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(Vec4F),
	},
};

static void onVkHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
}
static void onVkUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipeline->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

	vkCmdPushConstants(
		commandBuffer,
		pipeline->vk.layout,
		VK_SHADER_STAGE_VERTEX_BIT,
		0,
		sizeof(Mat4F),
		&handle->vk.mvp);
	vkCmdPushConstants(
		commandBuffer,
		pipeline->vk.layout,
		VK_SHADER_STAGE_FRAGMENT_BIT,
		0,
		sizeof(Vec3F),
		&handle->vk.color);
}
static void onVkHandleResize(
	Pipeline pipeline,
	void* createInfo)
{
	Vec2U framebufferSize = getWindowFramebufferSize(
		pipeline->vk.window);
	pipeline->vk.state.viewport = vec4I(0, 0,
		(int32_t)framebufferSize.x,
		(int32_t)framebufferSize.y);

	VkPipelineCreateInfo _createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		1,
		pushConstantRanges,
	};

	*(VkPipelineCreateInfo*)createInfo = _createInfo;
}

inline static Pipeline createVkHandle(
	Window window,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* handle)
{
	VkPipelineCreateInfo createInfo = {
		1,
		vertexInputBindingDescriptions,
		2,
		vertexInputAttributeDescriptions,
		0,
		NULL,
		1,
		pushConstantRanges,
	};

	return createPipeline(
		window,
		SPRITE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onVkHandleDestroy,
		NULL,
		onVkUniformsSet,
		onVkHandleResize,
		handle,
		&createInfo);
}
#endif

static void onGlHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
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
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.color);

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
		SPRITE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onGlHandleDestroy,
		NULL,
		onGlUniformsSet,
		onGlHandleResize,
		handle,
		NULL);

	if (pipeline == NULL)
		return NULL;

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation = getGlUniformLocation(
		glHandle,
		"u_MVP");

	if (mvpLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	GLint colorLocation = getGlUniformLocation(
		glHandle,
		"u_Color");

	if (colorLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.colorLocation = colorLocation;
	return pipeline;
}

Pipeline createExtSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	const PipelineState* state)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	uint8_t api = getWindowGraphicsAPI(window);

	Pipeline pipeline;

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		pipeline = createVkHandle(
			window,
			shaders,
			2,
			state,
			handle);
#else
		abort();
#endif
	}
	if (api == OPENGL_GRAPHICS_API ||
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

	handle->vk.mvp = identMat4F();
	handle->vk.color = oneVec4F();
	return pipeline;
}
Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
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

	return createExtSpritePipeline(
		window,
		vertexShader,
		fragmentShader,
		&state);
}

Mat4F getSpritePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setSpritePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

Vec4F getSpritePipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.color;
}
void setSpritePipelineColor(
	Pipeline pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(color.x >= 0.0f &&
		color.y >= 0.0f &&
		color.z >= 0.0f &&
		color.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.color = color;
}
