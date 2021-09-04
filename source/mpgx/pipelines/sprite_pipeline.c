#include "mpgx/pipelines/sprite_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkSpritePipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
} VkSpritePipeline;
typedef struct GlSpritePipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
} GlSpritePipeline;
typedef union SpritePipeline
{
	VkSpritePipeline vk;
	GlSpritePipeline gl;
} SpritePipeline;

inline static SpritePipeline* onGlSpritePipelineCreate(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	SpritePipeline* pipeline = malloc(
		sizeof(SpritePipeline));

	if (pipeline == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GLuint handle = createGlPipeline(
		window,
		shaders,
		2);

	if (handle == GL_ZERO)
	{
		free(pipeline);
		return NULL;
	}

	GLint mvpLocation = getGlUniformLocation(
		handle,
		"u_MVP");

	if (mvpLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint colorLocation = getGlUniformLocation(
		handle,
		"u_Color");

	if (colorLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.color = oneVec4F();
	pipeline->gl.handle = handle;
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	return pipeline;
}
static void onGlSpritePipelineDestroy(
	Window window,
	void* pipeline)
{
	SpritePipeline* handle =
		(SpritePipeline*)pipeline;
	destroyGlPipeline(
		window,
		handle->gl.handle);
	free(handle);
}
static void onGlSpritePipelineBind(
	Pipeline pipeline)
{
	Vec2U size = getWindowFramebufferSize(
		getPipelineWindow(pipeline));

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	SpritePipeline* handle =
		getPipelineHandle(pipeline);

	glUseProgram(handle->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glEnable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	glBlendFunc(
		GL_SRC_ALPHA,
		GL_ONE_MINUS_SRC_ALPHA);

	assertOpenGL();
}
static void onGlSpritePipelineUniformsSet(
	Pipeline pipeline)
{
	SpritePipeline* handle =
		getPipelineHandle(pipeline);

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
Pipeline createSpritePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	uint8_t api = getWindowGraphicsAPI(window);

	SpritePipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlSpritePipelineCreate(
			window,
			vertexShader,
			fragmentShader);

		onDestroy = onGlSpritePipelineDestroy;
		onBind = onGlSpritePipelineBind;
		onUniformsSet = onGlSpritePipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline pipeline = createPipeline(
		window,
		"Sprite",
		drawMode,
		onDestroy,
		onBind,
		onUniformsSet,
		handle);

	if (pipeline == NULL)
	{
		onDestroy(
			window,
			handle);
		return NULL;
	}

	return pipeline;
}

Shader getSpritePipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.vertexShader;
}
Shader getSpritePipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.fragmentShader;
}

Mat4F getSpritePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.mvp;
}
void setSpritePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.mvp = mvp;
}

Vec4F getSpritePipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.color;
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
		"Sprite") == 0);
	SpritePipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.color = color;
}
