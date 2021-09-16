#include "mpgx/pipelines/texcol_pipeline.h"
#include "mpgx/_source/opengl.h"

#include <string.h>

typedef struct VkPipelineHandle
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	Vec2F size;
	Vec2F offset;
} VkPipelineHandle;
typedef struct GlPipelineHandle
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
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

inline static PipelineHandle* createGlPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
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

	GLint sizeLocation = getGlUniformLocation(
		glHandle,
		"u_Size");

	if (sizeLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	GLint offsetLocation = getGlUniformLocation(
		glHandle,
		"u_Offset");

	if (offsetLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	GLint textureLocation = getGlUniformLocation(
		glHandle,
		"u_Texture");

	if (textureLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	assertOpenGL();

	pipelineHandle->gl.vertexShader = vertexShader;
	pipelineHandle->gl.fragmentShader = fragmentShader;
	pipelineHandle->gl.texture = texture;
	pipelineHandle->gl.sampler = sampler;
	pipelineHandle->gl.mvp = identMat4F();
	pipelineHandle->gl.color = oneVec4F();
	pipelineHandle->gl.size = oneVec2F();
	pipelineHandle->gl.offset = zeroVec2F();
	pipelineHandle->gl.handle = glHandle;
	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.colorLocation = colorLocation;
	pipelineHandle->gl.sizeLocation = sizeLocation;
	pipelineHandle->gl.offsetLocation = offsetLocation;
	pipelineHandle->gl.textureLocation = textureLocation;
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
	free(handle);
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
	glDisable(GL_BLEND);

	glColorMask(
		GL_TRUE, GL_TRUE,
		GL_TRUE, GL_TRUE);

	glDepthFunc(GL_LESS);
	glDepthMask(GL_TRUE);
	glDepthRange(0.0f, 1.0f);
	glPolygonOffset(0.0f, 0.0f);

	glFrontFace(GL_CW);
	glCullFace(GL_BACK);

	glUniform1i(
		pipelineHandle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	GLuint glTexture= *(const GLuint*)
		getImageHandle(pipelineHandle->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(pipelineHandle->gl.sampler);

	glBindTexture(
		GL_TEXTURE_2D,
		glTexture);
	glBindSampler(
		0,
		glSampler);

	assertOpenGL();
}
static void onGlPipelineUniformsSet(
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
	glUniform2fv(
		pipelineHandle->gl.sizeLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.size);
	glUniform2fv(
		pipelineHandle->gl.offsetLocation,
		1,
		(const GLfloat*)&pipelineHandle->gl.offset);

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
			fragmentShader,
			texture,
			sampler);

		onHandleDestroy = onGlPipelineHandleDestroy;
		onHandleBind = onGlPipelineHandleBind;
		onUniformsSet = onGlPipelineUniformsSet;
	}
	else
	{
		return NULL;
	}

	if (pipelineHandle == NULL)
		return NULL;

	Pipeline pipeline = createPipeline(
		window,
		TEX_COL_PIPELINE_NAME,
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

Shader getTexColPipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.vertexShader;
}
Shader getTexColPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.fragmentShader;
}
Image getTexColPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.texture;
}
Sampler getTexColPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.sampler;
}

Mat4F getTexColPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.mvp;
}
void setTexColPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.mvp = mvp;
}

Vec4F getTexColPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.color;
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
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.color = color;
}

Vec2F getTexColPipelineSize(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.size;
}
void setTexColPipelineSize(
	Pipeline pipeline,
	Vec2F size)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.size = size;
}

Vec2F getTexColPipelineOffset(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.offset;
}
void setTexColPipelineOffset(
	Pipeline pipeline,
	Vec2F offset)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		TEX_COL_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	pipelineHandle->vk.offset = offset;
}
