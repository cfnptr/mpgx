#include "mpgx/pipelines/gradsky_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/sampler.h"

#include <string.h>

struct GradSkyAmbient
{
	Vec4F* colors;
	size_t count;
};

typedef struct VkPipelineHandle
{
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	float sunHeight;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Image texture;
	Sampler sampler;
	Mat4F mvp;
	float sunHeight;
	GLint mvpLocation;
	GLint sunHeightLocation;
	GLint textureLocation;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

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
		NEVER_COMPARE_OPERATION,
		false,
		DEFAULT_MIN_MIPMAP_LOD,
		DEFAULT_MAX_MIPMAP_LOD,
		DEFAULT_MIPMAP_LOD_BIAS);
}

static void onGlHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle = handle;
	free(pipelineHandle);
}
static void onGlHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;

	glUniform1i(
		handle->gl.textureLocation,
		0);

	glActiveTexture(GL_TEXTURE0);

	glBindTexture(
		GL_TEXTURE_2D,
		handle->gl.texture->gl.handle);
	glBindSampler(
		0,
		handle->gl.sampler->gl.handle);

	assertOpenGL();
}
static void onGlUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle = pipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.mvp);
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
inline static Pipeline createGlHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
{
	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Vec2U framebufferSize =
		getWindowFramebufferSize(window);

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Pipeline pipeline = createPipeline(
		window,
		GRAD_SKY_PIPELINE_NAME,
		shaders,
		2,
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATION,
		ALL_COLOR_COMPONENT,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		vec4I(0, 0,
			(int32_t)framebufferSize.x,
			(int32_t)framebufferSize.y),
		vec2F(
			DEFAULT_MIN_DEPTH_RANGE,
			DEFAULT_MAX_DEPTH_RANGE),
		zeroVec4I(),
		onGlHandleDestroy,
		onGlHandleBind,
		onGlUniformsSet,
		NULL,
		handle,
		NULL);

	if (pipeline == NULL)
	{
		free(handle);
		return NULL;
	}

	GLuint glHandle = pipeline->gl.glHandle;

	GLint mvpLocation = getGlUniformLocation(
		glHandle,
		"u_MVP");

	if (mvpLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	GLint sunHeightLocation = getGlUniformLocation(
		glHandle,
		"u_SunHeight");

	if (sunHeightLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	GLint textureLocation = getGlUniformLocation(
		glHandle,
		"u_Texture");

	if (textureLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	assertOpenGL();

	handle->gl.texture = texture;
	handle->gl.sampler = sampler;
	handle->gl.mvp = identMat4F();
	handle->gl.mvp = identMat4F();
	handle->gl.sunHeight = 1.0f;
	handle->gl.mvpLocation = mvpLocation;
	handle->gl.sunHeightLocation = sunHeightLocation;
	handle->gl.textureLocation = textureLocation;
	return pipeline;
}

Pipeline createGradSkyPipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	Image texture,
	Sampler sampler)
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

	Pipeline pipeline;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipeline = createGlHandle(
			window,
			vertexShader,
			fragmentShader,
			texture,
			sampler);
	}
	else
	{
		return NULL;
	}

	if (pipeline == NULL)
		return NULL;

	return pipeline;
}

Image getGradSkyPipelineTexture(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.texture;
}
Sampler getGradSkyPipelineSampler(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.sampler;
}

Mat4F getGradSkyPipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setGradSkyPipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

float getGradSkyPipelineSunHeight(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.sunHeight;
}
void setGradSkyPipelineSunHeight(
	Pipeline pipeline,
	float sunHeight)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		GRAD_SKY_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.sunHeight = sunHeight;
}
