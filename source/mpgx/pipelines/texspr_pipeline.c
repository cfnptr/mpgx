#include "mpgx/pipelines/texspr_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

typedef struct VkPipelineHandle
{
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	GLint mvpLocation;
	GLint colorLocation;
	GLint sizeLocation;
	GLint offsetLocation;
	GLint textureLocation;
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
static void onGlHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle =
		pipeline->gl.handle;

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
		handle->gl.colorLocation,
		1,
		(const GLfloat*)&handle->gl.color);
	glUniform2fv(
		handle->gl.sizeLocation,
		1,
		(const GLfloat*)&handle->gl.size);
	glUniform2fv(
		handle->gl.offsetLocation,
		1,
		(const GLfloat*)&handle->gl.offset);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec2F) * 2,
		(const void*)sizeof(Vec2F));

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
		TEX_SPR_PIPELINE_NAME,
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

	GLint sizeLocation = getGlUniformLocation(
		glHandle,
		"u_Size");

	if (sizeLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	GLint offsetLocation = getGlUniformLocation(
		glHandle,
		"u_Offset");

	if (offsetLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	GLint textureLocation = getGlUniformLocation(
		glHandle,
		"u_Texture");

	if (textureLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		return NULL;
	}

	assertOpenGL();

	handle->gl.mvpLocation = mvpLocation;
	handle->gl.colorLocation = colorLocation;
	handle->gl.sizeLocation = sizeLocation;
	handle->gl.offsetLocation = offsetLocation;
	handle->gl.textureLocation = textureLocation;
	return pipeline;
}

Pipeline createExtTexSprPipeline(
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
	handle->vk.color = oneVec4F();
	handle->vk.size = oneVec2F();
	handle->vk.offset = zeroVec2F();
	return pipeline;
}
Pipeline createTexSprPipeline(
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

	return createExtTexSprPipeline(
		window,
		vertexShader,
		fragmentShader,
		texture,
		sampler,
		&state);
}

Image getTexSprPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.texture;
}
Sampler getTexSprPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.sampler;
}

Mat4F getTexSprPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setTexSprPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

Vec4F getTexSprPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.color;
}
void setTexSprPipelineColor(
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
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.color = color;
}

Vec2F getTexSprPipelineSize(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.size;
}
void setTexSprPipelineSize(
	Pipeline pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.size = size;
}

Vec2F getTexSprPipelineOffset(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.offset;
}
void setTexSprPipelineOffset(
	Pipeline pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_SPR_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.offset = offset;
}
