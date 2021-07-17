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
	Window window,
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
	Pipeline pipeline)
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
	Pipeline pipeline)
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
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.vertexShader;
}
Shader getTexColPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.fragmentShader;
}
Image getTexColPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexCol") == 0);
	TexColPipeline* texColPipeline =
		getPipelineHandle(pipeline);
	return texColPipeline->vk.texture;
}
Sampler getTexColPipelineSampler(
	Pipeline pipeline)
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
