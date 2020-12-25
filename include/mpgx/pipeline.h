#pragma once
#include "mpgx/window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>

inline static void assertOpenGL()
{
#ifndef NDEBUG
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
#endif
}

inline static GLuint createGlPipeline(
	struct Shader** _shaders,
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
	enum ImageFilter imageFilter,
	enum ImageFilter mipmapFilter,
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
inline static GLenum getGlImageWrap(
	enum ImageWrap wrap)
{
	if (wrap == CLAMP_TO_EDGE_IMAGE_WRAP)
		return GL_CLAMP_TO_EDGE;
	else if (wrap == MIRRORED_REPEAT_IMAGE_WRAP)
		return GL_MIRRORED_REPEAT;
	else
		return GL_REPEAT;
}

struct Pipeline* createColorPipeline(
	struct Window* window,
	struct Shader* vertexShader,
	struct Shader* fragmentShader,
	enum DrawMode drawMode);

struct Shader* getColorPipelineVertexShader(
	const struct Pipeline* pipeline);
struct Shader* getColorPipelineFragmentShader(
	const struct Pipeline* pipeline);

struct Matrix4F getColorPipelineMVP(
	const struct Pipeline* pipeline);
void setColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp);

struct Vector4F getColorPipelineColor(
	const struct Pipeline* pipeline);
void setColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color);

// TODO:
struct Pipeline* createImageColorPipeline(
	struct Window* window,
	enum DrawMode drawMode,
	enum CullFace cullFace,
	enum FrontFace frontFace,
	const void* vertexShader,
	size_t vertexShaderSize,
	const void* fragmentShader,
	size_t fragmentShaderSize,
	enum ImageFilter minFilter,
	enum ImageFilter magFilter,
	enum ImageFilter mipmapFilter,
	enum ImageWrap widthWrap,
	enum ImageWrap heightWrap,
	enum ImageWrap depthWrap,
	struct Image* image);

void setImageColorPipelineMVP(
	struct Pipeline* pipeline,
	struct Matrix4F mvp);
void setImageColorPipelineColor(
	struct Pipeline* pipeline,
	struct Vector4F color);
void setImageColorPipelineImage(
	struct Pipeline* pipeline,
	struct Image* image);

// TODO:
// 1. image filter/wrap gets
// 2. image offset/scale settings
