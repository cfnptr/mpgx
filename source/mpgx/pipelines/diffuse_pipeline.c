#include "mpgx/pipelines/diffuse_pipeline.h"
#include "mpgx/opengl.h"

#include <string.h>

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
	lightDirection = normVec3F(lightDirection);
	diffusePipeline->vk.fbo.lightDirection = vec4F(
		lightDirection.x,
		lightDirection.y,
		lightDirection.z,
		0.0f);
}
