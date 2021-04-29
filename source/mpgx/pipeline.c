#include "mpgx/pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkColorPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
	Vec4F color;
} VkColorPipeline;
typedef struct GlColorPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
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

typedef struct VkTexColPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* texture;
	Sampler* sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkTexColPipeline;
typedef struct GlTexColPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* texture;
	Sampler* sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint sizeLocation;
	GLint offsetLocation;
	GLint textureLocation;
} GlTexColPipeline;
typedef union TexColPipeline
{
	VkTexColPipeline vk;
	GlTexColPipeline gl;
} TexColPipeline;

typedef struct VkSpritePipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
	Vec4F color;
} VkSpritePipeline;
typedef struct GlSpritePipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
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

typedef struct VkTexSprPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* texture;
	Sampler* sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkTexSprPipeline;
typedef struct GlTexSprPipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Image* texture;
	Sampler* sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint sizeLocation;
	GLint offsetLocation;
	GLint textureLocation;
} GlTexSprPipeline;
typedef union TexSprPipeline
{
	VkTexSprPipeline vk;
	GlTexSprPipeline gl;
} TexSprPipeline;

typedef struct DiffuseUniformBuffer
{
	Vec4F objectColor;
	Vec4F ambientColor;
	Vec4F lightColor;
	Vec4F lightDirection;
} DiffuseUniformBuffer;
typedef struct VkDiffusePipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
	Mat4F normal;
	DiffuseUniformBuffer fbo;
} VkDiffusePipeline;
typedef struct GlDiffusePipeline
{
	Shader* vertexShader;
	Shader* fragmentShader;
	Mat4F mvp;
	Mat4F normal;
	DiffuseUniformBuffer fbo;
	GLuint handle;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer* uniformBuffer;
} GlDiffusePipeline;
typedef union DiffusePipeline
{
	VkDiffusePipeline vk;
	GlDiffusePipeline gl;
} DiffusePipeline;

inline static ColorPipeline* onGlColorPipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader)
{
	ColorPipeline* pipeline = malloc(
		sizeof(ColorPipeline));

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
	Window* window,
	void* pipeline)
{
	ColorPipeline* colorPipeline =
		(ColorPipeline*)pipeline;
	destroyGlPipeline(
		window,
		colorPipeline->gl.handle);
	free(colorPipeline);
}
static void onGlColorPipelineBind(
	Pipeline* pipeline)
{
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
	Pipeline* pipeline)
{
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		colorPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&colorPipeline->gl.mvp);
	glUniform4fv(
		colorPipeline->gl.colorLocation,
		1,
		(const GLfloat*)&colorPipeline->gl.color);

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
Pipeline* createColorPipeline(
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

	Pipeline* pipeline = createPipeline(
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

Shader* getColorPipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	return colorPipeline->vk.vertexShader;
}
Shader* getColorPipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	return colorPipeline->vk.fragmentShader;
}

Mat4F getColorPipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	return colorPipeline->vk.mvp;
}
void setColorPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	colorPipeline->vk.mvp = mvp;
}

Vec4F getColorPipelineColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	return colorPipeline->vk.color;
}
void setColorPipelineColor(
	Pipeline* pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Color") == 0);
	ColorPipeline* colorPipeline =
		getPipelineHandle(pipeline);
	colorPipeline->vk.color = color;
}

inline static TexColPipeline* onGlTexColPipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler)
{
	TexColPipeline* pipeline = malloc(
		sizeof(TexColPipeline));

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

	GLint sizeLocation = glGetUniformLocation(
		handle,
		"u_Size");

	if (sizeLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Size' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint offsetLocation = glGetUniformLocation(
		handle,
		"u_Offset");

	if (offsetLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Offset' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint textureLocation = glGetUniformLocation(
		handle,
		"u_Texture");

	if (textureLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Texture' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.texture = texture;
	pipeline->gl.sampler = sampler;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.color = oneVec4F();
	pipeline->gl.size = oneVec2F();
	pipeline->gl.offset = zeroVec2F();
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	pipeline->gl.sizeLocation = sizeLocation;
	pipeline->gl.offsetLocation = offsetLocation;
	pipeline->gl.textureLocation = textureLocation;
	return pipeline;
}
static void onGlTexColPipelineDestroy(
	Window* window,
	void* pipeline)
{
	TexColPipeline* texColPipeline =
		(TexColPipeline*)pipeline;
	destroyGlPipeline(
		window,
		texColPipeline->gl.handle);
	free(texColPipeline);
}
static void onGlTexColPipelineBind(
	Pipeline* pipeline)
{
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);

	glUseProgram(texColPipeline->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	GLuint glTexture= *(const GLuint*)
		getImageHandle(texColPipeline->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(texColPipeline->gl.sampler);

	glActiveTexture(GL_TEXTURE0 + 0);

	glBindTexture(
		GL_TEXTURE_2D,
		glTexture);
	glBindSampler(
		0,
		glSampler);
	glUniform1i(
		texColPipeline->gl.textureLocation,
		0);

	assertOpenGL();
}
static void onGlTexColPipelineUniformsSet(
	Pipeline* pipeline)
{
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		texColPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&texColPipeline->gl.mvp);
	glUniform4fv(
		texColPipeline->gl.colorLocation,
		1,
		(const GLfloat*)&texColPipeline->gl.color);
	glUniform2fv(
		texColPipeline->gl.sizeLocation,
		1,
		(const GLfloat*)&texColPipeline->gl.size);
	glUniform2fv(
		texColPipeline->gl.offsetLocation,
		1,
		(const GLfloat*)&texColPipeline->gl.offset);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) + sizeof(Vec2F),
		0);
	glVertexAttribPointer(
		1,
		2,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) + sizeof(Vec2F),
		(const void*)sizeof(Vec3F));

	assertOpenGL();
}
Pipeline* createTexColPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler,
	uint8_t drawMode)
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

	uint8_t api = getWindowGraphicsAPI(window);

	TexColPipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlTexColPipelineCreate(
			window,
			vertexShader,
			fragmentShader,
			texture,
			sampler);

		onDestroy = onGlTexColPipelineDestroy;
		onBind = onGlTexColPipelineBind;
		onUniformsSet = onGlTexColPipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline* pipeline = createPipeline(
		window,
		"TexCol",
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

Shader* getTexColPipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.vertexShader;
}
Shader* getTexColPipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.fragmentShader;
}
Image* getTexColPipelineTexture(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.texture;
}
Sampler* getTexColPipelineSampler(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.sampler;
}

Mat4F getTexColPipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.mvp;
}
void setTexColPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	texColPipeline->vk.mvp = mvp;
}

Vec4F getTexColPipelineColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.color;
}
void setTexColPipelineColor(
	Pipeline* pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	texColPipeline->vk.color = color;
}

Vec2F getTexColPipelineSize(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.size;
}
void setTexColPipelineSize(
	Pipeline* pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	texColPipeline->vk.size = size;
}

Vec2F getTexColPipelineOffset(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.offset;
}
void setTexColPipelineOffset(
	Pipeline* pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	texColPipeline->vk.offset = offset;
}

inline static SpritePipeline* onGlSpritePipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader)
{
	SpritePipeline* pipeline = malloc(
		sizeof(SpritePipeline));

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
	Window* window,
	void* pipeline)
{
	SpritePipeline* spritePipeline =
		(SpritePipeline*)pipeline;
	destroyGlPipeline(
		window,
		spritePipeline->gl.handle);
	free(spritePipeline);
}
static void onGlSpritePipelineBind(
	Pipeline* pipeline)
{
	SpritePipeline* spritePipeline =
		getPipelineHandle(pipeline);

	glUseProgram(spritePipeline->gl.handle);

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
	Pipeline* pipeline)
{
	SpritePipeline* spritePipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		spritePipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&spritePipeline->gl.mvp);
	glUniform4fv(
		spritePipeline->gl.colorLocation,
		1,
		(const GLfloat*)&spritePipeline->gl.color);

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
Pipeline* createSpritePipeline(
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

	Pipeline* pipeline = createPipeline(
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

Shader* getSpritePipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* spritePipeline =
		getPipelineHandle(pipeline);
	return spritePipeline->vk.vertexShader;
}
Shader* getSpritePipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* spritePipeline =
		getPipelineHandle(pipeline);
	return spritePipeline->vk.fragmentShader;
}

Mat4F getSpritePipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* spritePipeline =
		getPipelineHandle(pipeline);
	return spritePipeline->vk.mvp;
}
void setSpritePipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* colorPipeline =
		getPipelineHandle(pipeline);
	colorPipeline->vk.mvp = mvp;
}

Vec4F getSpritePipelineColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* colorPipeline =
		getPipelineHandle(pipeline);
	return colorPipeline->vk.color;
}
void setSpritePipelineColor(
	Pipeline* pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Sprite") == 0);
	SpritePipeline* colorPipeline =
		getPipelineHandle(pipeline);
	colorPipeline->vk.color = color;
}

inline static TexSprPipeline* onGlTexSprPipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler)
{
	TexSprPipeline* pipeline = malloc(
		sizeof(TexSprPipeline));

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

	GLint sizeLocation = glGetUniformLocation(
		handle,
		"u_Size");

	if (sizeLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Size' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint offsetLocation = glGetUniformLocation(
		handle,
		"u_Offset");

	if (offsetLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Offset' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint textureLocation = glGetUniformLocation(
		handle,
		"u_Texture");

	if (textureLocation == -1)
	{
#ifndef NDEBUG
		printf("Failed to get 'u_Texture' location\n");
#endif
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	assertOpenGL();

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.texture = texture;
	pipeline->gl.sampler = sampler;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.color = oneVec4F();
	pipeline->gl.size = oneVec2F();
	pipeline->gl.offset = zeroVec2F();
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	pipeline->gl.sizeLocation = sizeLocation;
	pipeline->gl.offsetLocation = offsetLocation;
	pipeline->gl.textureLocation = textureLocation;
	return pipeline;
}
static void onGlTexSprPipelineDestroy(
	Window* window,
	void* pipeline)
{
	TexSprPipeline* texSprPipeline =
		(TexSprPipeline*)pipeline;
	destroyGlPipeline(
		window,
		texSprPipeline->gl.handle);
	free(texSprPipeline);
}
static void onGlTexSprPipelineBind(
	Pipeline* pipeline)
{
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);

	glUseProgram(texSprPipeline->gl.handle);

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

	GLuint glTexture= *(const GLuint*)
		getImageHandle(texSprPipeline->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(texSprPipeline->gl.sampler);

	glActiveTexture(GL_TEXTURE0 + 0);

	glBindTexture(
		GL_TEXTURE_2D,
		glTexture);
	glBindSampler(
		0,
		glSampler);
	glUniform1i(
		texSprPipeline->gl.textureLocation,
		0);

	assertOpenGL();
}
static void onGlTexSprPipelineUniformsSet(
	Pipeline* pipeline)
{
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		texSprPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&texSprPipeline->gl.mvp);
	glUniform4fv(
		texSprPipeline->gl.colorLocation,
		1,
		(const GLfloat*)&texSprPipeline->gl.color);
	glUniform2fv(
		texSprPipeline->gl.sizeLocation,
		1,
		(const GLfloat*)&texSprPipeline->gl.size);
	glUniform2fv(
		texSprPipeline->gl.offsetLocation,
		1,
		(const GLfloat*)&texSprPipeline->gl.offset);

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
Pipeline* createTexSprPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	Image* texture,
	Sampler* sampler,
	uint8_t drawMode)
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

	uint8_t api = getWindowGraphicsAPI(window);

	TexSprPipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlTexSprPipelineCreate(
			window,
			vertexShader,
			fragmentShader,
			texture,
			sampler);

		onDestroy = onGlTexSprPipelineDestroy;
		onBind = onGlTexSprPipelineBind;
		onUniformsSet = onGlTexSprPipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline* pipeline = createPipeline(
		window,
		"TexSpr",
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

Shader* getTexSprPipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.vertexShader;
}
Shader* getTexSprPipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.fragmentShader;
}
Image* getTexSprPipelineTexture(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.texture;
}
Sampler* getTexSprPipelineSampler(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.sampler;
}

Mat4F getTexSprPipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.mvp;
}
void setTexSprPipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	texSprPipeline->vk.mvp = mvp;
}

Vec4F getTexSprPipelineColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.color;
}
void setTexSprPipelineColor(
	Pipeline* pipeline,
	Vec4F color)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	texSprPipeline->vk.color = color;
}

Vec2F getTexSprPipelineSize(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.size;
}
void setTexSprPipelineSize(
	Pipeline* pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	texSprPipeline->vk.size = size;
}

Vec2F getTexSprPipelineOffset(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.offset;
}
void setTexSprPipelineOffset(
	Pipeline* pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	texSprPipeline->vk.offset = offset;
}

inline static DiffusePipeline* onGlDiffusePipelineCreate(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader)
{
	DiffusePipeline* pipeline = malloc(
		sizeof(DiffusePipeline));

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

	Buffer* uniformBuffer = createBuffer(
		window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(DiffuseUniformBuffer),
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

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, 3.0f, 6.0f));

	pipeline->gl.vertexShader = vertexShader;
	pipeline->gl.fragmentShader = fragmentShader;
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.normal = identMat4F();
	pipeline->gl.fbo.objectColor = oneVec4F();
	pipeline->gl.fbo.ambientColor = valVec4F(0.5f);
	pipeline->gl.fbo.lightColor = oneVec4F();
	pipeline->gl.fbo.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.normalLocation = normalLocation;
	pipeline->gl.uniformBuffer = uniformBuffer;
	return pipeline;
}
static void onGlDiffusePipelineDestroy(
	Window* window,
	void* pipeline)
{
	DiffusePipeline* diffusePipeline =
		(DiffusePipeline*)pipeline;

	destroyBuffer(
		diffusePipeline->gl.uniformBuffer);
	destroyGlPipeline(
		window,
		diffusePipeline->gl.handle);
	free(diffusePipeline);
}
static void onGlDiffusePipelineBind(
	Pipeline* pipeline)
{
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);

	glUseProgram(diffusePipeline->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	Buffer* uniformBuffer =
		diffusePipeline->gl.uniformBuffer;
	GLuint buffer = *(GLuint*)
		getBufferHandle(uniformBuffer);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		buffer);

	assertOpenGL();

	setBufferData(
		uniformBuffer,
		&diffusePipeline->gl.fbo,
		sizeof(DiffuseUniformBuffer),
		0);
}
static void onGlDiffusePipelineUniformsSet(
	Pipeline* pipeline)
{
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		diffusePipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&diffusePipeline->gl.mvp);
	glUniformMatrix4fv(
		diffusePipeline->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&diffusePipeline->gl.normal);

	glEnableVertexAttribArray(0);
	glEnableVertexAttribArray(1);

	glVertexAttribPointer(
		0,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		0);
	glVertexAttribPointer(
		1,
		3,
		GL_FLOAT,
		GL_FALSE,
		sizeof(Vec3F) * 2,
		(const void*)sizeof(Vec3F));

	assertOpenGL();
}
Pipeline* createDiffusePipeline(
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

	DiffusePipeline* handle;
	OnPipelineDestroy onDestroy;
	OnPipelineBind onBind;
	OnPipelineUniformsSet onUniformsSet;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		handle = onGlDiffusePipelineCreate(
			window,
			vertexShader,
			fragmentShader);

		onDestroy = onGlDiffusePipelineDestroy;
		onBind = onGlDiffusePipelineBind;
		onUniformsSet = onGlDiffusePipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (handle == NULL)
		return NULL;

	Pipeline* pipeline = createPipeline(
		window,
		"Diffuse",
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

Shader* getDiffusePipelineVertexShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.vertexShader;
}
Shader* getDiffusePipelineFragmentShader(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.fragmentShader;
}

Mat4F getDiffusePipelineMVP(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.mvp;
}
void setDiffusePipelineMVP(
	Pipeline* pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.mvp = mvp;
}

Mat4F getDiffusePipelineNormal(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.normal;
}
void setDiffusePipelineNormal(
	Pipeline* pipeline,
	Mat4F normal)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.normal = normal;
}

Vec4F getDiffusePipelineObjectColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.fbo.objectColor;
}
void setDiffusePipelineObjectColor(
	Pipeline* pipeline,
	Vec4F objectColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.fbo.objectColor = objectColor;
}

Vec4F getDiffusePipelineAmbientColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.fbo.ambientColor;
}
void setDiffusePipelineAmbientColor(
	Pipeline* pipeline,
	Vec4F ambientColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.fbo.ambientColor = ambientColor;
}

Vec4F getDiffusePipelineLightColor(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	return diffusePipeline->vk.fbo.lightColor;
}
void setDiffusePipelineLightColor(
	Pipeline* pipeline,
	Vec4F lightColor)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.fbo.lightColor = lightColor;
}

Vec3F getDiffusePipelineLightDirection(
	const Pipeline* pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	Vec4F lightDirection =
		diffusePipeline->vk.fbo.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	Pipeline* pipeline,
	Vec3F lightDirection)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"Diffuse") == 0);
	DiffusePipeline* diffusePipeline =
		getPipelineHandle(pipeline);
	diffusePipeline->vk.fbo.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
