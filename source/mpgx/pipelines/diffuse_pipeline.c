#include "mpgx/pipelines/diffuse_pipeline.h"
#include "mpgx/_source/pipeline.h"
#include "mpgx/_source/buffer.h"

#include <string.h>

typedef struct UniformBuffer
{
	Vec4F objectColor;
	Vec4F ambientColor;
	Vec4F lightColor;
	Vec4F lightDirection;
} UniformBuffer;
typedef struct VkPipelineHandle
{
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

static void onGlPipelineHandleDestroy(void* handle)
{
	PipelineHandle* pipelineHandle =
		(PipelineHandle*)handle;
	destroyBuffer(
		pipelineHandle->gl.uniformBuffer);
	free(pipelineHandle);
}
static void onGlPipelineHandleBind(Pipeline pipeline)
{
	PipelineHandle* handle =
		pipeline->gl.handle;
	Buffer uniformBuffer =
		handle->gl.uniformBuffer;

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		uniformBuffer->gl.handle);

	assertOpenGL();

	setBufferData(
		uniformBuffer,
		&handle->gl.u,
		sizeof(UniformBuffer),
		0);
}
static void onGlPipelineUniformsSet(Pipeline pipeline)
{
	PipelineHandle* handle =
		pipeline->gl.handle;

	glUniformMatrix4fv(
		handle->gl.mvpLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.mvp);
	glUniformMatrix4fv(
		handle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&handle->gl.normal);

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
inline static Pipeline createGlPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	PipelineHandle* handle = malloc(
		sizeof(PipelineHandle));

	if (handle == NULL)
		return NULL;

	Shader shaders[2] = {
		vertexShader,
		fragmentShader,
	};

	Pipeline pipeline = createPipeline(
		window,
		DIFFUSE_PIPELINE_NAME,
		shaders,
		2,
		TRIANGLE_LIST_DRAW_MODE,
		FILL_POLYGON_MODE,
		BACK_CULL_MODE,
		LESS_COMPARE_OPERATION,
		true,
		true,
		true,
		true,
		false,
		false,
		false,
		DEFAULT_LINE_WIDTH,
		onGlPipelineHandleDestroy,
		onGlPipelineHandleBind,
		onGlPipelineUniformsSet,
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

	GLint normalLocation = getGlUniformLocation(
		glHandle,
		"u_Normal");

	if (normalLocation == GL_NULL_UNIFORM_LOCATION)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	GLuint uniformBlockIndex = getGlUniformBlockIndex(
		glHandle,
		"UniformBuffer");

	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	glUniformBlockBinding(
		glHandle,
		uniformBlockIndex,
		0);

	assertOpenGL();

	Buffer uniformBuffer = createBuffer(
		window,
		UNIFORM_BUFFER_TYPE,
		NULL,
		sizeof(UniformBuffer),
		false);

	if (uniformBuffer == NULL)
	{
		destroyPipeline(
			pipeline,
			false);
		free(handle);
		return NULL;
	}

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, -3.0f, 6.0f));

	handle->gl.mvp = identMat4F();
	handle->gl.normal = identMat4F();
	handle->gl.u.objectColor = oneVec4F();
	handle->gl.u.ambientColor = valVec4F(0.5f);
	handle->gl.u.lightColor = oneVec4F();
	handle->gl.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	handle->gl.mvpLocation = mvpLocation;
	handle->gl.normalLocation = normalLocation;
	handle->gl.uniformBuffer = uniformBuffer;
	return pipeline;
}

Pipeline createDiffusePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

	uint8_t api = getWindowGraphicsAPI(window);

	Pipeline pipeline;

	if (api == OPENGL_GRAPHICS_API ||
		api == OPENGL_ES_GRAPHICS_API)
	{
		pipeline = createGlPipelineHandle(
			window,
			vertexShader,
			fragmentShader);
	}
	else
	{
		return NULL;
	}

	if (pipeline == NULL)
		return NULL;

	return pipeline;
}

Mat4F getDiffusePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.mvp;
}
void setDiffusePipelineMvp(
	Pipeline pipeline,
	Mat4F mvp)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.mvp = mvp;
}

Mat4F getDiffusePipelineNormal(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.normal;
}
void setDiffusePipelineNormal(
	Pipeline pipeline,
	Mat4F normal)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.normal = normal;
}

Vec4F getDiffusePipelineObjectColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.objectColor;
}
void setDiffusePipelineObjectColor(
	Pipeline pipeline,
	Vec4F objectColor)
{
	assert(pipeline != NULL);
	assert(objectColor.x >= 0.0f &&
		objectColor.y >= 0.0f &&
		objectColor.z >= 0.0f &&
		objectColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.objectColor = objectColor;
}

Vec4F getDiffusePipelineAmbientColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.ambientColor;
}
void setDiffusePipelineAmbientColor(
	Pipeline pipeline,
	Vec4F ambientColor)
{
	assert(pipeline != NULL);
	assert(ambientColor.x >= 0.0f &&
		ambientColor.y >= 0.0f &&
		ambientColor.z >= 0.0f &&
		ambientColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.ambientColor = ambientColor;
}

Vec4F getDiffusePipelineLightColor(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	return pipelineHandle->vk.u.lightColor;
}
void setDiffusePipelineLightColor(
	Pipeline pipeline,
	Vec4F lightColor)
{
	assert(pipeline != NULL);
	assert(lightColor.x >= 0.0f &&
		lightColor.y >= 0.0f &&
		lightColor.z >= 0.0f &&
		lightColor.w >= 0.0f);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	pipelineHandle->vk.u.lightColor = lightColor;
}

Vec3F getDiffusePipelineLightDirection(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	Vec4F lightDirection =
		pipelineHandle->vk.u.lightDirection;
	return vec3F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z);
}
void setDiffusePipelineLightDirection(
	Pipeline pipeline,
	Vec3F lightDirection)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		pipeline->gl.handle;
	lightDirection = normVec3F(lightDirection);
	pipelineHandle->vk.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
