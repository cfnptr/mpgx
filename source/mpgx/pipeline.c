#include "mpgx/pipeline.h"

struct GlColorPipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
};
struct ColorPipeline
{
	struct Shader* vertexShader;
	struct Shader* fragmentShader;
	struct Matrix4F mvp;
	struct Vector4F color;
	void* handle;
};

inline static struct GlColorPipeline* createGlColorPipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader)
{
	struct GlColorPipeline* pipeline = malloc(
		sizeof(struct GlColorPipeline));

	if (pipeline == NULL)
		return NULL;

	struct Shader* shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	makeWindowContextCurrent(window);

	GLuint handle = createGlPipeline(
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

	GLint colorLocation = glGetUniformLocation(
		handle,
		"u_Color");

	if (colorLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Color' location\n");
#endif

		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	return pipeline;
}
void destroyGlColorPipeline(
	struct Window* window,
	void* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	makeWindowContextCurrent(window);

	glDeleteProgram(
		glColorPipeline->handle);

	assertOpenGL();

	free(glColorPipeline);
	free(colorPipeline);
}
void bindGlColorPipeline(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glUseProgram(glColorPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	assertOpenGL();
}
void setGlColorPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glUniformMatrix4fv(
		glColorPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&colorPipeline->mvp);
	glUniform4fv(
		glColorPipeline->colorLocation,
		1,
		(const GLfloat*)&colorPipeline->color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector3F),
		0);

	assertOpenGL();
}
struct Pipeline* createColorPipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);

	struct ColorPipeline* colorPipeline =
		malloc(sizeof(struct ColorPipeline));

	if (colorPipeline == NULL)
		return NULL;

	uint8_t api = getWindowGraphicsAPI(window);

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlColorPipeline(
			window,
			vertexShader,
			fragmentShader);

		destroyFunction = destroyGlColorPipeline;
		bindFunction = bindGlColorPipeline;
		setUniformsFunction = setGlColorPipelineUniforms;
	}
	else
	{
		free(colorPipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(colorPipeline);
		return NULL;
	}

	colorPipeline->vertexShader = vertexShader;
	colorPipeline->fragmentShader = fragmentShader;
	colorPipeline->mvp = createIdentityMatrix4F();
	colorPipeline->color = createValueVector4F(1.0f);
	colorPipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		destroyFunction,
		bindFunction,
		setUniformsFunction,
		colorPipeline);

	if (pipeline == NULL)
	{
		destroyGlColorPipeline(
			window,
			handle);
		
		free(colorPipeline);
		return NULL;
	}

	return pipeline;
}

struct Shader* getColorPipelineVertexShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->vertexShader;
}
struct Shader* getColorPipelineFragmentShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->fragmentShader;
}

struct Matrix4F getColorPipelineMVP(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->mvp;
}
void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	colorPipeline->mvp = mvp;
}

struct Vector4F getColorPipelineColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->color;
}
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	colorPipeline->color = color;
}
