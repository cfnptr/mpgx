#include "mpgx/pipelines/texspr_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkTexSprPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkTexSprPipeline;
typedef struct GlTexSprPipeline
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
} GlTexSprPipeline;
typedef union TexSprPipeline
{
	VkTexSprPipeline vk;
	GlTexSprPipeline gl;
} TexSprPipeline;

inline static TexSprPipeline* onGlTexSprPipelineCreate(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	TexSprPipeline* pipeline = malloc(
		sizeof(TexSprPipeline));

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
static void onGlTexSprPipelineDestroy(
	Window window,
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
	Pipeline pipeline)
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
	Pipeline pipeline)
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
Pipeline createTexSprPipeline(
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

	Pipeline pipeline = createPipeline(
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

Shader getTexSprPipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.vertexShader;
}
Shader getTexSprPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.fragmentShader;
}
Image getTexSprPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"TexSpr") == 0);
	TexSprPipeline* texSprPipeline =
		getPipelineHandle(pipeline);
	return texSprPipeline->vk.texture;
}
Sampler getTexSprPipelineSampler(
	Pipeline pipeline)
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
	Pipeline pipeline)
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
	Pipeline pipeline,
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
