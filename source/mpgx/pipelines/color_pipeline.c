#include "mpgx/pipelines/color_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkColorPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
} VkColorPipeline;
typedef struct GlColorPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
} GlColorPipeline;
typedef union ColorPipeline
{
	VkColorPipeline vk;
	GlColorPipeline gl;
} ColorPipeline;

inline static ColorPipeline* onGlColorPipelineCreate(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	ColorPipeline* pipeline = malloc(
		sizeof(ColorPipeline));

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
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	return pipeline;
}
static void onGlColorPipelineDestroy(
	Window window,
	void* pipeline)
{
	ColorPipeline* handle =
		(ColorPipeline*)pipeline;
	destroyGlPipeline(
		window,
		handle->gl.handle);
	free(handle);
}
static void onGlColorPipelineBind(
	Pipeline pipeline)
{
	Vec2U size = getWindowFramebufferSize(
		getPipelineWindow(pipeline));

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);

	glUseProgram(colorPipeline->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	assertOpenGL();
}
static void onGlColorPipelineUniformsSet(
	Pipeline pipeline)
{
	ColorPipeline* handle =
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
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F),
		0);

	assertOpenGL();
}
Pipeline createColorPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	uint8_t api = getWindowGraphicsAPI(window);

	ColorPipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlColorPipelineCreate(
			window,
			vertexShader,
			fragmentShader);

		onDestroy = onGlColorPipelineDestroy;
		onBind = onGlColorPipelineBind;
		onUniformsSet = onGlColorPipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline pipeline = createPipeline(
		window,
		"Color",
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

Shader getColorPipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.vertexShader;
}
Shader getColorPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.fragmentShader;
}

Mat4F getColorPipelineMVP(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.mvp;
}
void setColorPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.mvp = mvp;
}

Vec4F getColorPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.color;
}
void setColorPipelineColor(
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
		"Color") == 0);
	ColorPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.color = color;
}
