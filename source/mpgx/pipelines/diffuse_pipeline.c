#include "mpgx/pipelines/diffuse_pipeline.h"
#include "mpgx/opengl.h"

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
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
} VkPipelineHandle;
typedef struct GlPipelineHandle
{
	Shader vertexShader;
	Shader fragmentShader;
	Mat4F mvp;
	Mat4F normal;
	UniformBuffer u;
	GLuint handle;
	GLint mvpLocation;
	GLint normalLocation;
	Buffer uniformBuffer;
} GlPipelineHandle;
typedef union PipelineHandle
{
	VkPipelineHandle vk;
	GlPipelineHandle gl;
} PipelineHandle;

inline static PipelineHandle* createGlPipelineHandle(
	Window window,
	Shader vertexShader,
	Shader fragmentShader)
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

	GLint normalLocation = getGlUniformLocation(
		glHandle,
		"u_Normal");

	if (normalLocation == NULL_UNIFORM_LOCATION)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	GLuint uniformBlockIndex = getGlUniformBlockIndex(
		glHandle,
		"UniformBuffer");

	if (uniformBlockIndex == GL_INVALID_INDEX)
	{
		glDeleteProgram(glHandle);
		free(pipelineHandle);
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
		glDeleteProgram(glHandle);
		free(pipelineHandle);
		return NULL;
	}

	Vec3F lightDirection = normVec3F(
		vec3F(1.0f, 3.0f, 6.0f));

	pipelineHandle->gl.vertexShader = vertexShader;
	pipelineHandle->gl.fragmentShader = fragmentShader;
	pipelineHandle->gl.mvp = identMat4F();
	pipelineHandle->gl.normal = identMat4F();
	pipelineHandle->gl.u.objectColor = oneVec4F();
	pipelineHandle->gl.u.ambientColor = valVec4F(0.5f);
	pipelineHandle->gl.u.lightColor = oneVec4F();
	pipelineHandle->gl.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
	pipelineHandle->gl.handle = glHandle;
	pipelineHandle->gl.mvpLocation = mvpLocation;
	pipelineHandle->gl.normalLocation = normalLocation;
	pipelineHandle->gl.uniformBuffer = uniformBuffer;
	return pipelineHandle;
}
static void onGlPipelineHandleDestroy(
	Window window,
	void* handle)
{
	PipelineHandle* pipelineHandle =
		(PipelineHandle*)handle;
	destroyBuffer(
		pipelineHandle->gl.uniformBuffer);
	destroyGlPipeline(
		window,
		pipelineHandle->gl.handle);
	free(pipelineHandle);
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

	Buffer uniformBuffer =
		pipelineHandle->gl.uniformBuffer;
	GLuint glBuffer = *(GLuint*)
		getBufferHandle(uniformBuffer);

	glBindBufferBase(
		GL_UNIFORM_BUFFER,
		0,
		glBuffer);

	assertOpenGL();

	setBufferData(
		uniformBuffer,
		&pipelineHandle->gl.u,
		sizeof(UniformBuffer),
		0);
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
	glUniformMatrix4fv(
		pipelineHandle->gl.normalLocation,
		1,
		GL_FALSE,
		(const GLfloat*)&pipelineHandle->gl.normal);

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
Pipeline createDiffusePipeline(
	Window window,
	Shader vertexShader,
	Shader fragmentShader,
	uint8_t drawMode)
{
	assert(window != NULL);
	assert(vertexShader != NULL);
	assert(fragmentShader != NULL);
	assert(drawMode < DRAW_MODE_COUNT);
	assert(getShaderType(vertexShader) == VERTEX_SHADER_TYPE);
	assert(getShaderType(fragmentShader) == FRAGMENT_SHADER_TYPE);
	assert(getShaderWindow(vertexShader) == window);
	assert(getShaderWindow(fragmentShader) == window);

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
			fragmentShader);

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
		DIFFUSE_PIPELINE_NAME,
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

Shader getDiffusePipelineVertexShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.vertexShader;
}
Shader getDiffusePipelineFragmentShader(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
	return pipelineHandle->vk.fragmentShader;
}

Mat4F getDiffusePipelineMvp(
	Pipeline pipeline)
{
	assert(pipeline != NULL);
	assert(strcmp(
		getPipelineName(pipeline),
		DIFFUSE_PIPELINE_NAME) == 0);
	PipelineHandle* pipelineHandle =
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
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
		getPipelineHandle(pipeline);
	lightDirection = normVec3F(lightDirection);
	pipelineHandle->vk.u.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
