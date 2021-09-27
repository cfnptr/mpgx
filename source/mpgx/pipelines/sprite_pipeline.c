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
inline static Pipeline createGlHandle(
	Window window,
	Shader* shaders,
	uint8_t shaderCount,
	const PipelineState* state,
	PipelineHandle* handle)
{
	// TODO: enable blending
	Pipeline pipeline = createPipeline(
		window,
		SPRITE_PIPELINE_NAME,
		shaders,
		shaderCount,
		state,
		onGlHandleDestroy,
		NULL,
		onGlUniformsSet,
		NULL,
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
		free(handle);
		return NULL;
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

	PipelineState state = {
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATION,
		ALL_COLOR_COMPONENT,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		vec4I(0, 0,
			(int32_t)framebufferSize.x,
			(int32_t)framebufferSize.y),
		vec2F(
			DEFAULT_MIN_DEPTH_RANGE,
			DEFAULT_MAX_DEPTH_RANGE),
		zeroVec4I(),
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
