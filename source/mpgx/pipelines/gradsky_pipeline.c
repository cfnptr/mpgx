#include "mpgx/pipelines/gradsky_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct GlGradSkyPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
	GLuint handle;
	GLint mvpLocation;
} GlGradSkyPipeline;
typedef struct VkGradSkyPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
} VkGradSkyPipeline;
typedef union GradSkyPipeline
{
	VkGradSkyPipeline vk;
	GlGradSkyPipeline gl;
} GradSkyPipeline;

inline static GradSkyPipeline* onGlGradSkyPipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader)
{
	GradSkyPipeline* pipeline = malloc(
		sizeof(GradSkyPipeline));

	if (pipeline == NULL)
		return NULL;

	Shader* shaders[2] = {
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

	GLint mvpLocation = glGetUniformLocation(
		handle,
		"u_MVP");

	if (mvpLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_MVP' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	return pipeline;
}
static void onGlGradSkyPipelineDestroy(
	Window* window,
	void* pipeline)
{
	GradSkyPipeline* gradSkyPipeline =
		(GradSkyPipeline*)pipeline;
	destroyGlPipeline(
		window,
		gradSkyPipeline->gl.handle);
	free(gradSkyPipeline);
}
static void onGlGradSkyPipelineBind(
	Pipeline* pipeline)
{
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);

	glUseProgram(gradSkyPipeline->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	assertOpenGL();
}
static void onGlGradSkyPipelineUniformsSet(
	Pipeline* pipeline)
{
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		gradSkyPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&gradSkyPipeline->gl.mvp);

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
Pipeline* createGradSkyPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
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

	GradSkyPipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlGradSkyPipelineCreate(
			window,
			vertexShader,
			fragmentShader);

		onDestroy = onGlGradSkyPipelineDestroy;
		onBind = onGlGradSkyPipelineBind;
		onUniformsSet = onGlGradSkyPipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline* pipeline = createPipeline(
		window,
		"GradSky",
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

Shader* getGradSkyPipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.vertexShader;
}
Shader* getGradSkyPipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.fragmentShader;
}

Mat4F getGradSkyPipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.mvp;
}
void setGradSkyPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	gradSkyPipeline->vk.mvp = mvp;
}
