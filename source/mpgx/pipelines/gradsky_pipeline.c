#include "mpgx/pipelines/gradsky_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

struct GradSkyAmbient
{
	Vec4F* colors;
	size_t count;
};

typedef struct VkGradSkyPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	float sunHeight;
} VkGradSkyPipeline;
typedef struct GlGradSkyPipeline
{
	Shader vertexShader;
	Shader fragmentShader;
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	Vec4F color;
	float sunHeight;
	GLuint handle;
	GLint mvpLocation;
	GLint colorLocation;
	GLint sunHeightLocation;
	GLint textureLocation;
} GlGradSkyPipeline;
typedef union GradSkyPipeline
{
	VkGradSkyPipeline vk;
	GlGradSkyPipeline gl;
} GradSkyPipeline;

GradSkyAmbient createGradSkyAmbient(
	ImageData gradient)
{
	assert(gradient != NULL);
	assert(getImageDataChannelCount(gradient) == 4);

	Vec2U size = getImageDataSize(gradient);

	Vec4F* colors = malloc(
		sizeof(Vec4F) * size.x);

	if (colors == NULL)
		return NULL;

	const uint8_t* pixels = getImageDataPixels(gradient);

	for (uint32_t x = 0; x < size.x; x++)
	{
		Vec4F color = zeroVec4F();

		for (uint32_t y = 0; y < size.y; y++)
		{
			size_t index = (y * size.x + x) * 4;
			
			Vec4F addition = vec4F(
				(float)pixels[index] / 255.0f,
				(float)pixels[index + 1] / 255.0f,
				(float)pixels[index + 2] / 255.0f,
				(float)pixels[index + 3] / 255.0f);
			color = addVec4F(color, addition);
		}

		colors[x] = divValVec4F(color, (float)size.y);
	}

	GradSkyAmbient gradSkyAmbient = malloc(
		sizeof(struct GradSkyAmbient));

	if (gradSkyAmbient == NULL)
	{
		free(colors);
		return NULL;
	}

	gradSkyAmbient->colors = colors;
	gradSkyAmbient->count = size.x;
	return gradSkyAmbient;
}
void destroyGradSkyAmbient(
	GradSkyAmbient gradSkyAmbient)
{
	if (gradSkyAmbient == NULL)
		return;

	free(gradSkyAmbient->colors);
	free(gradSkyAmbient);
}
Vec4F getGradSkyAmbientColor(
	GradSkyAmbient gradSkyAmbient,
	float dayTime)
{
	assert(gradSkyAmbient != NULL);
	assert(dayTime >= 0.0f);
	assert(dayTime <= 1.0f);

	Vec4F* colors = gradSkyAmbient->colors;
	size_t colorCount = gradSkyAmbient->count;

	dayTime = (float)(colorCount - 1) * dayTime;

	float secondValue = dayTime - (float)((int)dayTime);
	float firstValue = 1.0f - secondValue;

	Vec4F firstColor = colors[(size_t)dayTime];
	Vec4F secondColor = colors[(size_t)dayTime + 1];

	return vec4F(
		firstColor.x * firstValue + secondColor.x * secondValue,
		firstColor.y * firstValue + secondColor.y * secondValue,
		firstColor.z * firstValue + secondColor.z * secondValue,
		firstColor.w * firstValue + secondColor.w * secondValue);
}

Sampler createGradSkySampler(Window window)
{
	assert(window != NULL);

	return createSampler(
		window,
		LINEAR_IMAGE_FILTER,
		LINEAR_IMAGE_FILTER,
		NEAREST_IMAGE_FILTER,
		false,
		CLAMP_TO_EDGE_IMAGE_WRAP,
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

	GLint sunHeightLocation = getGlUniformLocation(
		handle,
		"u_SunHeight");

	if (sunHeightLocation == NULL_UNIFORM_LOCATION)
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
	pipeline->gl.sunHeight = 1.0f;
	pipeline->gl.handle = handle;
	pipeline->gl.mvpLocation = mvpLocation;
	pipeline->gl.colorLocation = colorLocation;
	pipeline->gl.sunHeightLocation = sunHeightLocation;
	pipeline->gl.textureLocation = textureLocation;
	return pipeline;
}
static void onGlGradSkyPipelineDestroy(
	Window window,
	void* pipeline)
{
	GradSkyPipeline* handle =
		(GradSkyPipeline*)pipeline;
	destroyGlPipeline(
		window,
		handle->gl.handle);
	free(handle);
}
static void onGlGradSkyPipelineBind(
	Pipeline pipeline)
{
	Vec2U size = getWindowFramebufferSize(
		getPipelineWindow(pipeline));

	glViewport(
		0,
		0,
		(GLsizei)size.x,
		(GLsizei)size.y);

	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);

	glUseProgram(handle->gl.handle);

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
		getImageHandle(handle->gl.texture);
	GLuint glSampler = *(const GLuint*)
		getSamplerHandle(handle->gl.sampler);

	glActiveTexture(GL_TEXTURE0 + 0);

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
static void onGlGradSkyPipelineUniformsSet(
	Pipeline pipeline)
{
	GradSkyPipeline* handle =
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
	glUniform1fv(
		handle->gl.sunHeightLocation,
		1,
		(const GLfloat*)&handle->gl.sunHeight);

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
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.vertexShader;
}
Shader getGradSkyPipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.fragmentShader;
}
Image getGradSkyPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.texture;
}
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.sampler;
}

Mat4F getGradSkyPipelineMVP(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.mvp;
}
void setGradSkyPipelineMVP(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.mvp = mvp;
}

Vec4F getGradSkyPipelineColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.color;
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
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.color = color;
}

float getGradSkyPipelineSunHeight(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	return handle->vk.sunHeight;
}
void setGradSkyPipelineSunHeight(
	Pipeline pipeline,
	float sunHeight)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		"GradSky") == 0);
	GradSkyPipeline* handle =
		getPipelineHandle(pipeline);
	handle->vk.sunHeight = sunHeight;
}
