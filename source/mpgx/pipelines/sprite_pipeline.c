#include "mpgx/pipelines/sprite_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkPipelineHandle
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Vec4F color;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

inline static PipelineHandle* createGlPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	PipelineHandle* pipelineHandle = malloc(
		sizeof(PipelineHandle));

	if (pipelineHandle == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	GLuint glHandle = createGlPipeline(
		window,
		shaders,
		2);

	if (glHandle == GL_ZERO)
	{
		free(pipelineHandle);
		return NULL;
	}

	GLint mvpLocation = getGlUniformLocation(
		glHandle,
		"u_MVP");

	if (mvpLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	GLint colorLocation = getGlUniformLocation(
		glHandle,
		"u_Color");

	if (colorLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.vertexShader = vertexShader;
	pipelineHandle->gl.fragmentShader = fragmentShader;
	pipelineHandle->gl.mvp = identMat4F();
	pipelineHandle->gl.color = oneVec4F();
	pipelineHandle->gl.handle = glHandle;
	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.colorLocation = colorLocation;
	return pipelineHandle;
}
static void onGlPipelineHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle =
		(PipelineHandle*)handle;
	destroyGlPipeline(
		window,
		pipelineHandle->gl.handle);
	free(pipelineHandle);
}
static void onGlPipelineHandleBind(
	Pipeline pipeline)
{
	Vec2U size = getWindowFramebufferSize(
		getPipelineWindow(pipeline));

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);

	glUseProgram(pipelineHandle->gl.handle);

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
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		pipelineHandle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.mvp);
	glUniform4fv(
		pipelineHandle->gl.colorLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.color);

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

	PipelineHandle* pipelineHandle;
	OnPipelineHandleDestroy onHandleDestroy;
	OnPipelineHandleBind onHandleBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipelineHandle = createGlPipelineHandle(
			window,
			vertexShader,
			fragmentShader);

		onHandleDestroy = onGlPipelineHandleDestroy;
		onHandleBind = onGlPipelineHandleBind;
		onUniformsSet = onGlSpritePipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (pipelineHandle == NULL)
		return NULL;

	Pipeline pipeline = createPipeline(
		window,
		SPRITE_PIPELINE_NAME,
		drawMode,
		onHandleDestroy,
		onHandleBind,
		onUniformsSet,
		pipelineHandle);

	if (pipeline == NULL)
	{
		onHandleDestroy(
			window,
			pipelineHandle);
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
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.vertexShader;
}
Shader getSpritePipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.fragmentShader;
}

Mat4F getSpritePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		SPRITE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
	pipelineHandle->vk.color = color;
}
