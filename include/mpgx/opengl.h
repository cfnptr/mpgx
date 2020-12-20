#pragma once
#include "mpgx/window.h"

#include "glad/glad.h"
#include "GLFW/glfw3.h"

#include <stdio.h>
#include <assert.h>

#define OPENGL_SHADER_HEADER "#version 330 core\n\n#define highp \n#define mediump \n#define lowp \n"
#define OPENGL_ES_SHADER_HEADER "#version 300 es\n"

// TODO: read shaders from files

#define OPENGL_COLOR_VERTEX_SHADER                 \
"layout(location = 0) in highp vec3 v_Position;\n  \
uniform highp mat4 u_MVP;\n                        \
                                                   \
void main()\n                                      \
{\n                                                \
	gl_Position = u_MVP * vec4(v_Position, 1.0);\n \
}\n"
#define OPENGL_COLOR_FRAGMENT_SHADER \
"out highp vec4 o_Color;\n           \
uniform highp vec4 u_Color;\n        \
                                     \
void main()\n                        \
{\n                                  \
	o_Color = u_Color;\n             \
}\n"

#define OPENGL_TEXT_VERTEX_SHADER                       \
"layout(location = 0) in highp vec2 v_Position;\n       \
layout(location = 1) in highp vec2 v_TexCoord;\n        \
out highp vec2 f_TexCoord;\n                            \
                                                        \
uniform highp mat4 u_MVP;\n                             \
                                                        \
void main()\n                                           \
{\n                                                     \
	gl_Position = u_MVP * vec4(v_Position, 0.0, 1.0);\n \
	f_TexCoord = vec2(v_TexCoord.x, -v_TexCoord.y);\n   \
}\n"
#define OPENGL_TEXT_FRAGMENT_SHADER                    \
"in highp vec2 f_TexCoord;\n                           \
out highp vec4 o_Color;\n                              \
                                                       \
uniform highp vec4 u_Color;\n                          \
uniform sampler2D u_Image;\n                           \
                                                       \
void main()\n                                          \
{\n                                                    \
	vec4 sample = texture(u_Image, f_TexCoord);\n      \
	o_Color = sample * u_Color;\n                      \
}\n"

inline static void assertOpenGL()
{
#ifndef NDEBUG
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();
#endif
}
inline static GLuint createGlShader(
	GLenum stage,
	const char* source,
	bool gles)
{
	GLuint shader = glCreateShader(stage);

	const char* sources[2];

	if (gles == false)
		sources[0] = OPENGL_SHADER_HEADER;
	else
		sources[0] = OPENGL_ES_SHADER_HEADER;

	sources[1] = source;

	glShaderSource(
		shader,
		2,
		sources,
		NULL);

	glCompileShader(shader);

	GLint result;

	glGetShaderiv(
		shader,
		GL_COMPILE_STATUS,
		&result);

	if (result == GL_FALSE)
	{
#ifndef NDEBUG
		GLint length = 0;

		glGetShaderiv(
			shader,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog =
				malloc(length * sizeof(char));

			if (infoLog == NULL)
				abort();

			glGetShaderInfoLog(
				shader,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteShader(shader);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	return shader;
}
inline static GLuint createGlPipeline(
	const GLenum* shaderStages,
	const char** shaderSources,
	size_t shaderCount,
	bool gles)
{
	assert(shaderStages != NULL);
	assert(shaderSources != NULL);
	assert(shaderCount != 0);

	GLuint* shaders = malloc(
		shaderCount * sizeof(GLuint));

	if (shaders == NULL)
		return GL_ZERO;

	GLuint program = glCreateProgram();

	for (size_t i = 0; i < shaderCount; i++)
	{
		shaders[i] = createGlShader(
			shaderStages[i],
			shaderSources[i],
			gles);
	}

	for (size_t i = 0; i < shaderCount; i++)
	{
		glAttachShader(
			program,
			shaders[i]);
	}

	glLinkProgram(program);

	for (size_t i = 0; i < shaderCount; i++)
	{
		glDetachShader(
			program,
			shaders[i]);
	}

	for (size_t i = 0; i < shaderCount; i++)
		glDeleteShader(shaders[i]);

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
				abort();

			glGetProgramInfoLog(
				program,
				length,
				&length,
				infoLog);

			printf("%s\n", infoLog);
		}
#endif

		GLenum error = glGetError();

		if (error != GL_NO_ERROR)
			abort();

		glDeleteProgram(program);
		free(shaders);
		return GL_ZERO;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		abort();

	free(shaders);
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
inline static GLenum getGlImageType(
	enum ImageType type)
{
	if (type == IMAGE_3D_TYPE)
		return GL_TEXTURE_3D;
	else
		return GL_TEXTURE_2D;
}
