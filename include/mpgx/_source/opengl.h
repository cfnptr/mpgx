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

inline static bool getGlImageFilter(
	uint8_t imageFilter,
	uint8_t mipmapFilter,
	bool useMipmapping,
	GLenum* glImageFilter)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		if (useMipmapping == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
			{
				*glImageFilter = GL_NEAREST_MIPMAP_NEAREST;
				return true;
			}
			else if (mipmapFilter == LINEAR_IMAGE_FILTER)
			{
				*glImageFilter = GL_NEAREST_MIPMAP_LINEAR;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			*glImageFilter = GL_NEAREST;
			return true;
		}
	}
	else if (imageFilter == LINEAR_IMAGE_FILTER)
	{
		if (useMipmapping == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
			{
				*glImageFilter = GL_LINEAR_MIPMAP_NEAREST;
				return true;
			}
			else if (mipmapFilter == LINEAR_IMAGE_FILTER)
			{
				*glImageFilter = GL_LINEAR_MIPMAP_LINEAR;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			*glImageFilter = GL_LINEAR;
			return true;
		}
	}
	else
	{
		return false;
	}
}
inline static bool getGlImageWrap(
	uint8_t imageWrap,
	GLenum* glImageWrap)
{
	if (imageWrap == CLAMP_TO_EDGE_IMAGE_WRAP)
	{
		*glImageWrap = GL_CLAMP_TO_EDGE;
		return true;
	}
	else if (imageWrap == MIRRORED_REPEAT_IMAGE_WRAP)
	{
		*glImageWrap = GL_MIRRORED_REPEAT;
		return true;
	}
	else if (imageWrap == REPEAT_IMAGE_WRAP)
	{
		*glImageWrap = GL_REPEAT;
		return true;
	}
	else
	{
		return false;
	}
}
inline static bool getGlImageCompare(
	uint8_t imageCompare,
	GLenum* glImageCompare)
{
	switch (imageCompare)
	{
	default:
		return false;
	case LESS_IMAGE_COMPARE:
		*glImageCompare = GL_LESS;
		return true;
	case LESS_OR_EQUAL_IMAGE_COMPARE:
		*glImageCompare = GL_LEQUAL;
		return true;
	case GREATER_OR_EQUAL_IMAGE_COMPARE:
		*glImageCompare = GL_GEQUAL;
		return true;
	case GREATER_IMAGE_COMPARE:
		*glImageCompare = GL_GREATER;
		return true;
	case EQUAL_IMAGE_COMPARE:
		*glImageCompare = GL_EQUAL;
		return true;
	case NOT_EQUAL_IMAGE_COMPARE:
		*glImageCompare = GL_NOTEQUAL;
		return true;
	case ALWAYS_IMAGE_COMPARE:
		*glImageCompare = GL_ALWAYS;
		return true;
	case NEVER_IMAGE_COMPARE:
		*glImageCompare = GL_NEVER;
		return true;
	}
}
