#include "mpgx/pipeline.h"

// TODO: combine GlPipeline with Pipeline
// if statements faster
// todo fix diffuse normals, not works correctly

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
	struct Mat4F mvp;
	struct Vec4F color;
	void* handle;
};

struct GlSpritePipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint colorLocation;
};
struct SpritePipeline
{
	struct Shader* vertexShader;
	struct Shader* fragmentShader;
	struct Mat4F mvp;
	struct Vec4F color;
	void* handle;
};

struct GlDiffusePipeline
{
	GLenum handle;
	GLint mvpLocation;
	GLint normalLocation;
	struct Buffer* uniformBuffer;
};
struct DiffuseUniformBuffer
{
	struct Vec4F objectColor;
	struct Vec4F ambientColor;
	struct Vec4F lightColor;
	struct Vec4F lightDirection;
};
struct DiffusePipeline
{
	struct Shader* vertexShader;
	struct Shader* fragmentShader;
	struct Mat4F mvp;
	struct Mat4F normal;
	struct DiffuseUniformBuffer fbo;
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
static void destroyGlColorPipeline(
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
static void bindGlColorPipeline(
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
static void setGlColorPipelineUniforms(
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
		sizeof(struct Vec3F),
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
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	struct ColorPipeline* colorPipeline = malloc(
		sizeof(struct ColorPipeline));

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
	colorPipeline->mvp = identMat4F();
	colorPipeline->color = valVec4F(1.0f);
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
		destroyFunction(
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

struct Mat4F getColorPipelineMVP(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->mvp;
}
void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Mat4F mvp)
{
	assert(pipeline != NULL);
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	colorPipeline->mvp = mvp;
}

struct Vec4F getColorPipelineColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	return colorPipeline->color;
}
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vec4F color)
{
	assert(pipeline != NULL);
	struct ColorPipeline* colorPipeline =
		(struct ColorPipeline*)getPipelineHandle(pipeline);
	colorPipeline->color = color;
}

inline static struct GlSpritePipeline* createGlSpritePipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader)
{
	struct GlSpritePipeline* pipeline = malloc(
		sizeof(struct GlSpritePipeline));

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
static void destroyGlSpritePipeline(
	struct Window* window,
	void* pipeline)
{
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)pipeline;
	struct GlSpritePipeline* glSpritePipeline =
		(struct GlSpritePipeline*)spritePipeline->handle;

	makeWindowContextCurrent(window);

	glDeleteProgram(
		glSpritePipeline->handle);

	assertOpenGL();

	free(glSpritePipeline);
	free(spritePipeline);
}
static void bindGlSpritePipeline(
	struct Pipeline* pipeline)
{
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	struct GlSpritePipeline* glSpritePipeline =
		(struct GlSpritePipeline*)spritePipeline->handle;

	glUseProgram(glSpritePipeline->handle);

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
static void setGlSpritePipelineUniforms(
	struct Pipeline* pipeline)
{
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	struct GlSpritePipeline* glSpritePipeline =
		(struct GlSpritePipeline*)spritePipeline->handle;

	glUniformMatrix4fv(
		glSpritePipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&spritePipeline->mvp);
	glUniform4fv(
		glSpritePipeline->colorLocation,
		1,
		(const GLfloat*)&spritePipeline->color);

	glEnableVertexAttribArray(0);

	glVertexAttribPointer(
		0,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vec2F),
		0);

	assertOpenGL();
}
struct Pipeline* createSpritePipeline(
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
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	struct SpritePipeline* spritePipeline = malloc(
		sizeof(struct SpritePipeline));

	if (spritePipeline == NULL)
		return NULL;

	uint8_t api = getWindowGraphicsAPI(window);

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlSpritePipeline(
			window,
			vertexShader,
			fragmentShader);

		destroyFunction = destroyGlSpritePipeline;
		bindFunction = bindGlSpritePipeline;
		setUniformsFunction = setGlSpritePipelineUniforms;
	}
	else
	{
		free(spritePipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(spritePipeline);
		return NULL;
	}

	spritePipeline->vertexShader = vertexShader;
	spritePipeline->fragmentShader = fragmentShader;
	spritePipeline->mvp = identMat4F();
	spritePipeline->color = valVec4F(1.0f);
	spritePipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		destroyFunction,
		bindFunction,
		setUniformsFunction,
		spritePipeline);

	if (pipeline == NULL)
	{
		destroyFunction(
			window,
			handle);

		free(spritePipeline);
		return NULL;
	}

	return pipeline;
}

struct Shader* getSpritePipelineVertexShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	return spritePipeline->vertexShader;
}
struct Shader* getSpritePipelineFragmentShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	return spritePipeline->fragmentShader;
}

struct Mat4F getSpritePipelineMVP(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct SpritePipeline* spritePipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	return spritePipeline->mvp;
}
void setSpritePipelineMVP(
	struct Pipeline* pipeline,
	struct Mat4F mvp)
{
	assert(pipeline != NULL);
	struct SpritePipeline* colorPipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	colorPipeline->mvp = mvp;
}

struct Vec4F getSpritePipelineColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct SpritePipeline* colorPipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	return colorPipeline->color;
}
void setSpritePipelineColor(
	struct Pipeline* pipeline,
	struct Vec4F color)
{
	assert(pipeline != NULL);
	struct SpritePipeline* colorPipeline =
		(struct SpritePipeline*)getPipelineHandle(pipeline);
	colorPipeline->color = color;
}

inline static struct GlDiffusePipeline* createGlDiffusePipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader)
{
	struct GlDiffusePipeline* pipeline = malloc(
		sizeof(struct GlDiffusePipeline));

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

	GLint normalLocation = glGetUniformLocation(
		handle,
		"u_Normal");

	if (normalLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Normal' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLuint uniformBlockIndex = glGetUniformBlockIndex(
		handle,
		"FragmentBufferObject");

	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
#ifndef NDEBUG
		printf("Failed to get 'FragmentBufferObject' block index\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	glUniformBlockBinding(
		handle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	struct Buffer* uniformBuffer = createBuffer(
		window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(struct DiffuseUniformBuffer),
		false);

	if (uniformBuffer == NULL)
	{
#ifndef NDEBUG
		printf("Failed to create diffuse uniform buffer\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	pipeline->handle = handle;
	pipeline->mvpLocation = mvpLocation;
	pipeline->normalLocation = normalLocation;
	pipeline->uniformBuffer = uniformBuffer;
	return pipeline;
}
static void destroyGlDiffusePipeline(
	struct Window* window,
	void* pipeline)
{
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)pipeline;
	struct GlDiffusePipeline* glDiffusePipeline =
		(struct GlDiffusePipeline*)diffusePipeline->handle;

	destroyBuffer(glDiffusePipeline->uniformBuffer);
	glDeleteProgram(glDiffusePipeline->handle);

	assertOpenGL();

	free(glDiffusePipeline);
	free(diffusePipeline);
}
static void bindGlDiffusePipeline(
	struct Pipeline* pipeline)
{
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	struct GlDiffusePipeline* glDiffusePipeline =
		(struct GlDiffusePipeline*)diffusePipeline->handle;

	glUseProgram(glDiffusePipeline->handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	struct Buffer* uniformBuffer =
		glDiffusePipeline->uniformBuffer;
	GLuint buffer = *(GLuint*)
		getBufferHandle(uniformBuffer);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		buffer);

	assertOpenGL();

	setBufferData(
		uniformBuffer,
		&diffusePipeline->fbo,
		sizeof(struct DiffuseUniformBuffer),
		0);
}
static void setGlDiffusePipelineUniforms(
	struct Pipeline* pipeline)
{
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	struct GlDiffusePipeline* glDiffusePipeline =
		(struct GlDiffusePipeline*)diffusePipeline->handle;

	glUniformMatrix4fv(
		glDiffusePipeline->mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&diffusePipeline->mvp);
	glUniformMatrix4fv(
		glDiffusePipeline->normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&diffusePipeline->normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vec3F) * 2,
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(struct Vec3F) * 2,
		(const void*)sizeof(struct Vec3F));

	assertOpenGL();
}
struct Pipeline* createDiffusePipeline(
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
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	struct DiffusePipeline* diffusePipeline = malloc(
		sizeof(struct DiffusePipeline));

	if (diffusePipeline == NULL)
		return NULL;

	uint8_t api = getWindowGraphicsAPI(window);

	void* handle;

	DestroyPipeline destroyFunction;
	BindPipelineCommand bindFunction;
	SetUniformsCommand setUniformsFunction;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = createGlDiffusePipeline(
			window,
			vertexShader,
			fragmentShader);

		destroyFunction = destroyGlDiffusePipeline;
		bindFunction = bindGlDiffusePipeline;
		setUniformsFunction = setGlDiffusePipelineUniforms;
	}
	else
	{
		free(diffusePipeline);
		return NULL;
	}

	if (handle == NULL)
	{
		free(diffusePipeline);
		return NULL;
	}

	struct Vec3F lightDirection = normVec3F(
		vec3F(1.0f, 2.0f, 3.0f));

	diffusePipeline->vertexShader = vertexShader;
	diffusePipeline->fragmentShader = fragmentShader;
	diffusePipeline->mvp = identMat4F();
	diffusePipeline->normal = identMat4F();
	diffusePipeline->fbo.objectColor = valVec4F(1.0f);
	diffusePipeline->fbo.ambientColor = valVec4F(0.5f);
	diffusePipeline->fbo.lightColor = valVec4F(1.0f);
	diffusePipeline->fbo.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	diffusePipeline->handle = handle;

	struct Pipeline* pipeline = createPipeline(
		window,
		drawMode,
		destroyFunction,
		bindFunction,
		setUniformsFunction,
		diffusePipeline);

	if (pipeline == NULL)
	{
		destroyFunction(
			window,
			handle);

		free(diffusePipeline);
		return NULL;
	}

	return pipeline;
}

struct Shader* getDiffusePipelineVertexShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->vertexShader;
}
struct Shader* getDiffusePipelineFragmentShader(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->fragmentShader;
}

struct Mat4F getDiffusePipelineMVP(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->mvp;
}
void setDiffusePipelineMVP(
	struct Pipeline* pipeline,
	struct Mat4F mvp)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	diffusePipeline->mvp = mvp;
}

struct Mat4F getDiffusePipelineNormal(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->normal;
}
void setDiffusePipelineNormal(
	struct Pipeline* pipeline,
	struct Mat4F normal)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	diffusePipeline->normal = normal;
}

struct Vec4F getDiffusePipelineObjectColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->fbo.objectColor;
}
void setDiffusePipelineObjectColor(
	struct Pipeline* pipeline,
	struct Vec4F objectColor)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	diffusePipeline->fbo.objectColor = objectColor;
}

struct Vec4F getDiffusePipelineAmbientColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->fbo.ambientColor;
}
void setDiffusePipelineAmbientColor(
	struct Pipeline* pipeline,
	struct Vec4F ambientColor)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	diffusePipeline->fbo.ambientColor = ambientColor;
}

struct Vec4F getDiffusePipelineLightColor(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	return diffusePipeline->fbo.lightColor;
}
void setDiffusePipelineLightColor(
	struct Pipeline* pipeline,
	struct Vec4F lightColor)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	diffusePipeline->fbo.lightColor = lightColor;
}

struct Vec3F getDiffusePipelineLightDirection(
	const struct Pipeline* pipeline)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);
	struct Vec4F lightDirection =
		diffusePipeline->fbo.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	struct Pipeline* pipeline,
	struct Vec3F lightDirection)
{
	assert(pipeline != NULL);
	struct DiffusePipeline* diffusePipeline =
		(struct DiffusePipeline*)getPipelineHandle(pipeline);

	diffusePipeline->fbo.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
