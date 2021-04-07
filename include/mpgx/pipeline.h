#pragma once
#include "mpgx/window.h"
#include "mpgx/defines.h"

#if MPGX_VULKAN_SUPPORT
#define GLFW_INCLUDE_VULKAN
#endif

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>
#include <stdbool.h>

inline static void assertOpenGL()
{
#ifndef NDEBUG
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
#endif
}

inline static GLuint createGlPipeline(
	Shader** _shaders,
	size_t shaderCount)
{
	assert(_shaders != NULL);
	assert(shaderCount != 0);

	GLuint program = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		GLuint handle = *(const GLuint*)
			getShaderHandle(_shaders[i]);

		glAttachShader(
			program,
			handle);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		GLuint handle = *(const GLuint*)
			getShaderHandle(_shaders[i]);

		glDetachShader(
			program,
			handle);
	}

	GLint result;

	glGetProgramiv(
		program,
		GL_LINK_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetProgramiv(
			program,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteProgram(program);
				return GL_ZERO;
			}

			glGetProgramInfoLog(
				program,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		assertOpenGL();

		glDeleteProgram(program);
		return GL_ZERO;
	}

	assertOpenGL();

	return program;
}

inline static GLenum getGlImageFilter(
	uint8_t imageFilter,
	uint8_t mipmapFilter,
	bool mipmap)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		if (mipmap == true)
		{
			if(mipmapFilter == NEAREST_IMAGE_FILTER)
				return GL_NEAREST_MIPMAP_NEAREST;
			else
				return GL_NEAREST_MIPMAP_LINEAR;
		}
		else
		{
			return GL_NEAREST;
		}
	}
	else
	{
		if (mipmap == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
				return GL_LINEAR_MIPMAP_NEAREST;
			else
				return GL_LINEAR_MIPMAP_LINEAR;
		}
		else
		{
			return GL_LINEAR;
		}
	}
}
inline static GLenum getGlImageWrap(uint8_t wrap)
{
	if (wrap == CLAMP_TO_EDGE_IMAGE_WRAP)
		return GL_CLAMP_TO_EDGE;
	else if (wrap == MIRRORED_REPEAT_IMAGE_WRAP)
		return GL_MIRRORED_REPEAT;
	else
		return GL_REPEAT;
}

Pipeline* createColorPipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getColorPipelineVertexShader(
	const Pipeline* pipeline);
Shader* getColorPipelineFragmentShader(
	const Pipeline* pipeline);

Matrix4F getColorPipelineMVP(
	const Pipeline* pipeline);
void setColorPipelineMVP(
	Pipeline* pipeline,
	Matrix4F mvp);

Vector4F getColorPipelineColor(
	const Pipeline* pipeline);
void setColorPipelineColor(
	Pipeline* pipeline,
	Vector4F color);

Pipeline* createSpritePipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getSpritePipelineVertexShader(
	const Pipeline* pipeline);
Shader* getSpritePipelineFragmentShader(
	const Pipeline* pipeline);

Matrix4F getSpritePipelineMVP(
	const Pipeline* pipeline);
void setSpritePipelineMVP(
	Pipeline* pipeline,
	Matrix4F mvp);

Vector4F getSpritePipelineColor(
	const Pipeline* pipeline);
void setSpritePipelineColor(
	Pipeline* pipeline,
	Vector4F color);

Pipeline* createDiffusePipeline(
	Window* window,
	Shader* vertexShader,
	Shader* fragmentShader,
	uint8_t drawMode);

Shader* getDiffusePipelineVertexShader(
	const Pipeline* pipeline);
Shader* getDiffusePipelineFragmentShader(
	const Pipeline* pipeline);

Matrix4F getDiffusePipelineMVP(
	const Pipeline* pipeline);
void setDiffusePipelineMVP(
	Pipeline* pipeline,
	Matrix4F mvp);

Matrix4F getDiffusePipelineNormal(
	const Pipeline* pipeline);
void setDiffusePipelineNormal(
	Pipeline* pipeline,
	Matrix4F normal);

Vector4F getDiffusePipelineObjectColor(
	const Pipeline* pipeline);
void setDiffusePipelineObjectColor(
	Pipeline* pipeline,
	Vector4F objectColor);

Vector4F getDiffusePipelineAmbientColor(
	const Pipeline* pipeline);
void setDiffusePipelineAmbientColor(
	Pipeline* pipeline,
	Vector4F ambientColor);

Vector4F getDiffusePipelineLightColor(
	const Pipeline* pipeline);
void setDiffusePipelineLightColor(
	Pipeline* pipeline,
	Vector4F lightColor);

Vector3F getDiffusePipelineLightDirection(
	const Pipeline* pipeline);
void setDiffusePipelineLightDirection(
	Pipeline* pipeline,
	Vector3F lightDirection);
