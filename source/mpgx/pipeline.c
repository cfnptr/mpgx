#include "mpgx/pipeline.h"
#include "mpgx/opengl.h"

struct GlColorPipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
};
struct ColorPipeline
{
	struct Matrix4F mvp;
	struct Vector4F color;
	void* handle;
};

struct GlImageColorPipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint imageLocation;
};
struct ImageColorPipeline
{
	enum ImageFilter minFilter;
	enum ImageFilter magFilter;
	enum ImageFilter mipmapFilter;
	enum ImageWrap widthWrap;
	enum ImageWrap heightWrap;
	enum ImageWrap depthWrap;
	struct Matrix4F mvp;
	struct Vector4F color;
	struct Image* image;
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
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline->handle;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	makeWindowContextCurrent(
		pipeline->window);

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
		(struct ColorPipeline*)pipeline->handle;
	struct GlColorPipeline* glColorPipeline =
		(struct GlColorPipeline*)colorPipeline->handle;

	glUseProgram(glColorPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	assertOpenGL();
}
void setGlColorPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline->handle;
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
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);

	struct ColorPipeline* colorPipeline =
		malloc(sizeof(struct ColorPipeline));

	if (colorPipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		getWindowGraphicsAPI(window);

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

	colorPipeline->mvp = createIdentityMatrix4F();
	colorPipeline->color = createValueVector4F(1.0f);
	colorPipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		destroyFunction,
		NULL,
		bindFunction,
		colorPipeline);

	if (pipeline == NULL)
	{
		destroyGlColorPipeline(handle);
		free(colorPipeline);
		return NULL;
	}

	return pipeline;
}

struct Matrix4F getColorPipelineMVP(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	return colorPipeline->mvp;
}
void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	colorPipeline->mvp = mvp;
}

struct Vector4F getColorPipelineColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	return colorPipeline->color;
}
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color)
{
	assert(pipeline != NULL);

	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)pipeline;
	colorPipeline->color = color;
}

inline static struct GlImageColorPipeline* createGlImageColorPipeline(
	struct Window* window,
	const void* vertexShader,
	const void* fragmentShader,
	bool gles)
{
	struct GlImageColorPipeline* pipeline = malloc(
		sizeof(struct GlImageColorPipeline));

	if (pipeline == NULL)
		return NULL;

	GLenum stages[2] = {
		GL_VERTEX_SHADER,
		GL_FRAGMENT_SHADER,
	};
	const char* shaders[2] = {
		(const char*)vertexShader,
		(const char*)fragmentShader,
	};

	makeWindowContextCurrent(window);

	// TODO:
	/*GLuint handle = createGlPipeline(
		stages,
		shaders,
		2,
		gles);

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

	GLint imageLocation = glGetUniformLocation(
		handle,
		"u_Image");

	if (imageLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Image' location\n");
#endif

		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->colorLocation = colorLocation;
	pipeline->imageLocation = imageLocation;*/
	return pipeline;
}
void destroyGlImageColorPipeline(
	struct Pipeline* pipeline)
{
	struct ImageColorPipeline* imageColorPipeline =
		(struct ImageColorPipeline*)pipeline->handle;
	struct GlImageColorPipeline* glImageColorPipeline =
		(struct GlImageColorPipeline*)imageColorPipeline->handle;

	makeWindowContextCurrent(
		pipeline->window);

	glDeleteProgram(
		glImageColorPipeline->handle);

	assertOpenGL();

	free(glImageColorPipeline);
	free(imageColorPipeline);
}
void bindGlImageColorPipeline(
	struct Pipeline* pipeline)
{
	struct ImageColorPipeline* imageColorPipeline =
		(struct ImageColorPipeline*)pipeline->handle;
	struct GlImageColorPipeline* glImageColorPipeline =
		(struct GlImageColorPipeline*)imageColorPipeline->handle;

	glUseProgram(glImageColorPipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	// TODO: bind image and set wrap/filter

	assertOpenGL();
}
void setGlImageColorPipelineUniforms(
	struct Pipeline* pipeline)
{
	struct ImageColorPipeline* imageColorPipeline =
		(struct ImageColorPipeline*)pipeline->handle;
	struct GlImageColorPipeline* glImageColorPipeline =
		(struct GlImageColorPipeline*)imageColorPipeline->handle;

	glUniformMatrix4fv(
		glImageColorPipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&imageColorPipeline->mvp);
	glUniform4fv(
		glImageColorPipeline->colorLocation,
		1,
		(const GLfloat*)&imageColorPipeline->color);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector3F),
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vector2F),
		(const void*)sizeof(struct Vector3F));

	assertOpenGL();
}
struct Pipeline* createImageColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize,
	enum ImageFilter minFilter,
	enum ImageFilter magFilter,
	enum ImageFilter mipmapFilter,
	enum ImageWrap widthWrap,
	enum ImageWrap heightWrap,
	enum ImageWrap depthWrap,
	struct Image* image)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(image != NULL);
	assert(window == getImageWindow(image));

	struct ImageColorPipeline* imageColorPipeline =
		malloc(sizeof(struct ImageColorPipeline));

	if (imageColorPipeline == NULL)
		return NULL;

	enum GraphicsAPI api =
		getWindowGraphicsAPI(window);;

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API)
	{
		handle = createGlImageColorPipeline(
			window,
			vertexShader,
			fragmentShader,
			false);

		destroyFunction = destroyGlColorPipeline;
		bindFunction = bindGlColorPipeline;
		setUniformsFunction = setGlColorPipelineUniforms;
	}
	else if (api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlImageColorPipeline(
			window,
			vertexShader,
			fragmentShader,
			true);

		destroyFunction = destroyGlColorPipeline;
		bindFunction = bindGlColorPipeline;
		setUniformsFunction = setGlColorPipelineUniforms;
	}
	else
	{
		free(imageColorPipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(imageColorPipeline);
		return NULL;
	}
	imageColorPipeline->minFilter = minFilter;
	imageColorPipeline->magFilter = magFilter;
	imageColorPipeline->mipmapFilter = mipmapFilter;
	imageColorPipeline->widthWrap = widthWrap;
	imageColorPipeline->heightWrap = heightWrap;
	imageColorPipeline->depthWrap = depthWrap;
	imageColorPipeline->mvp = createIdentityMatrix4F();
	imageColorPipeline->color = createValueVector4F(1.0f);
	imageColorPipeline->image = image;
	imageColorPipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		destroyFunction,
		NULL,
		bindFunction,
		imageColorPipeline);

	if (pipeline == NULL)
	{
		destroyGlColorPipeline(handle);
		free(imageColorPipeline);
		return NULL;
	}

	return pipeline;
}

// TODO:
void setImageColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp)
{

}
void setImageColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color)
{

}
void setImageColorPipelineImage(
	struct Pipeline* pipeline,
	struct Image* image)
{

}
