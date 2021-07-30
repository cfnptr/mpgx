#include "mpgx/pipelines/texcol_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkTexColPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkTexColPipeline;
typedef struct GlTexColPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
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

inline static TexColPipeline* onGlTexColPipelineCreate(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	TexColPipeline* pipeline = malloc(
		sizeof(TexColPipeline));

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

	GLint sizeLocation = getGlUniformLocation(
		handle,
		"u_Size");

	if (sizeLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint offsetLocation = getGlUniformLocation(
		handle,
		"u_Offset");

	if (offsetLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(handle);
		free(pipeline);
		return NULL;
	}

	GLint textureLocation = getGlUniformLocation(
		handle,
		"u_Texture");

	if (textureLocation == NULL_UNIFORM_LOCATION)
	{
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
	Window window,
	void* pipeline)
{
	TexColPipeline* handle =
		(TexColPipeline*)pipeline;
	destroyGlPipeline(
		window,
		handle->gl.handle);
	free(handle);
}
static void onGlTexColPipelineBind(
	Pipeline pipeline)
{
	Vec2U size = getWindowFramebufferSize(
		getPipelineWindow(pipeline));

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	TexColPipeline* handle =
		getPipelineHandle(pipeline);

	glUseProgram(handle->gl.handle);

	glEnable(GL_DEPTH_TEST);
	glEnable(GL_CULL_FACE);
	glDisable(GL_SCISSOR_TEST);
	glDisable(GL_STENCIL_TEST);
	glDisable(GL_BLEND);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	GLuint glTexture= *(const GLuint*)
		getImageHandle(handle->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(handle->gl.sampler);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		glTexture);
	glBindSampler(
		0,
		glSampler);
	glUniform1i(
		handle->gl.textureLocation,
		0);

	assertOpenGL();
}
static void onGlTexColPipelineUniformsSet(
	Pipeline pipeline)
{
	TexColPipeline* handle =
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
Pipeline createTexColPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(texture != NULL);
	assert(sampler != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
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

	Pipeline pipeline = createPipeline(
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

Shader getTexColPipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.vertexShader;
}
Shader getTexColPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.fragmentShader;
}
Image getTexColPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.texture;
}
Sampler getTexColPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.sampler;
}

Mat4F getTexColPipelineMVP(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.mvp;
}
void setTexColPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.mvp = mvp;
}

Vec4F getTexColPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.color;
}
void setTexColPipelineColor(
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
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.color = color;
}

Vec2F getTexColPipelineSize(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.size;
}
void setTexColPipelineSize(
	Pipeline pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.size = size;
}

Vec2F getTexColPipelineOffset(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.offset;
}
void setTexColPipelineOffset(
	Pipeline pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.offset = offset;
}
