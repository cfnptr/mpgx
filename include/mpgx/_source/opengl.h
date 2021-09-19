#pragma once
#include "mpgx/_source/graphics.h"

#ifndef GL_INFO_LOG_OUT
#define GL_INFO_LOG_OUT stderr
#endif

#include <stdio.h>

inline static void assertOpenGL()
{
#ifndef NDEBUG
	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		const char* errorName;

		switch (error)
		{
		default:
			errorName = "UNKNOWN_ERROR";
			break;
		case GL_INVALID_ENUM:
			errorName = "GL_INVALID_ENUM";
			break;
		case GL_INVALID_VALUE:
			errorName = "GL_INVALID_VALUE";
			break;
		case GL_INVALID_OPERATION:
			errorName = "GL_INVALID_OPERATION";
			break;
		case GL_STACK_OVERFLOW:
			errorName = "GL_STACK_OVERFLOW";
			break;
		case GL_STACK_UNDERFLOW:
			errorName = "GL_STACK_UNDERFLOW";
			break;
		case GL_OUT_OF_MEMORY:
			errorName = "GL_OUT_OF_MEMORY";
			break;
		case GL_INVALID_FRAMEBUFFER_OPERATION:
			errorName = "GL_INVALID_FRAMEBUFFER_OPERATION";
			break;
		case GL_CONTEXT_LOST:
			errorName = "GL_CONTEXT_LOST";
			break;
		}

		fprintf(stderr,
			"OpenGL error: %s\n",
			errorName);

		abort();
	}
#endif
}

inline static GLenum getGlImageFilter(
	uint8_t imageFilter,
	uint8_t mipmapFilter,
	bool useMipmapping)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		if (useMipmapping == true)
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
		if (useMipmapping == true)
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
inline static GLenum getGlImageCompare(uint8_t compare)
{
	switch (compare)
	{
	default:
	case LESS_IMAGE_COMPARE:
		return GL_LESS;
	case LESS_EQUAL_IMAGE_COMPARE:
		return GL_LEQUAL;
	case GREATER_EQUAL_IMAGE_COMPARE:
		return GL_GEQUAL;
	case GREATER_IMAGE_COMPARE:
		return GL_GREATER;
	case EQUAL_IMAGE_COMPARE:
		return GL_EQUAL;
	case NOT_EQUAL_IMAGE_COMPARE:
		return GL_NOTEQUAL;
	case ALWAYS_IMAGE_COMPARE:
		return GL_ALWAYS;
	case NEVER_IMAGE_COMPARE:
		return GL_NEVER;
	}
}
