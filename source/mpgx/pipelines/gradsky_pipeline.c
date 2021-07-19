#include "mpgx/pipelines/gradsky_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

typedef struct VkGradSkyPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	float time;
} VkGradSkyPipeline;
typedef struct GlGradSkyPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	float time;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint timeLocation;
	GLint textureLocation;
} GlGradSkyPipeline;
typedef union GradSkyPipeline
{
	VkGradSkyPipeline vk;
	GlGradSkyPipeline gl;
} GradSkyPipeline;

Sampler createGradSkySampler(Window window)
{
	assert(window != NULL);

	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		false,
		REPEAT_IMAGE_WRAP,
		CLAMP_TO_EDGE_IMAGE_WRAP,
		REPEAT_IMAGE_WRAP,
		NEVER_IMAGE_COMPARE,
		false,
		DEFAULT_MIN_MIPMAP_LOD,
		DEFAULT_MAX_MIPMAP_LOD);
}

inline static GradSkyPipeline* onGlGradSkyPipelineCreate(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	GradSkyPipeline* pipeline = malloc(
		sizeof(GradSkyPipeline));

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

	GLint timeLocation = getGlUniformLocation(
		handle,
		"u_Time");

	if (timeLocation == NULL_UNIFORM_LOCATION)
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
	pipeline->gl.mvp = identMat4F();
	pipeline->gl.color = oneVec4F();
	pipeline->gl.time = 0.5f;
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	pipeline->gl.timeLocation = timeLocation;
	pipeline->gl.textureLocation = textureLocation;
	return pipeline;
}
static void onGlGradSkyPipelineDestroy(
	Window window,
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
	Pipeline pipeline)
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

	GLuint glTexture= *(const GLuint*)
		getImageHandle(gradSkyPipeline->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(gradSkyPipeline->gl.sampler);

	glActiveTexture(GL_TEXTURE0 + 0);

	glBindTexture(
		GL_TEXTURE_2D,
		glTexture);
	glBindSampler(
		0,
		glSampler);
	glUniform1i(
		gradSkyPipeline->gl.textureLocation,
		0);

	assertOpenGL();
}
static void onGlGradSkyPipelineUniformsSet(
	Pipeline pipeline)
{
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);

	glUniformMatrix4fv(
		gradSkyPipeline->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&gradSkyPipeline->gl.mvp);
	glUniform4fv(
		gradSkyPipeline->gl.colorLocation,
		1,
		(const GLfloat*)&gradSkyPipeline->gl.color);
	glUniform1fv(
		gradSkyPipeline->gl.timeLocation,
		1,
		(const GLfloat*)&gradSkyPipeline->gl.time);

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
Pipeline createGradSkyPipeline(
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
			fragmentShader,
			texture,
			sampler);

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

	Pipeline pipeline = createPipeline(
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

Shader getGradSkyPipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.vertexShader;
}
Shader getGradSkyPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.fragmentShader;
}
Image getGradSkyPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.texture;
}
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.sampler;
}

Mat4F getGradSkyPipelineMVP(
	Pipeline pipeline)
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
	Pipeline pipeline,
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

Vec4F getGradSkyPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.color;
}
void setGradSkyPipelineColor(
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
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	gradSkyPipeline->vk.color = color;
}

float getGradSkyPipelineTime(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	return gradSkyPipeline->vk.time;
}
void setGradSkyPipelineTime(
	Pipeline pipeline,
	float time)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* gradSkyPipeline =
		getPipelineHandle(pipeline);
	gradSkyPipeline->vk.time = time;
}
