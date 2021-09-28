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

inline static bool getGlCompareOperation(
	uint8_t compareOperation,
	GLenum* glCompareOperation)
{
	switch (compareOperation)
	{
	default:
		return false;
	case LESS_COMPARE_OPERATOR:
		*glCompareOperation = GL_LESS;
		return true;
	case LESS_OR_EQUAL_COMPARE_OPERATOR:
		*glCompareOperation = GL_LEQUAL;
		return true;
	case GREATER_OR_EQUAL_COMPARE_OPERATOR:
		*glCompareOperation = GL_GEQUAL;
		return true;
	case GREATER_COMPARE_OPERATOR:
		*glCompareOperation = GL_GREATER;
		return true;
	case EQUAL_COMPARE_OPERATOR:
		*glCompareOperation = GL_EQUAL;
		return true;
	case NOT_EQUAL_COMPARE_OPERATOR:
		*glCompareOperation = GL_NOTEQUAL;
		return true;
	case ALWAYS_COMPARE_OPERATOR:
		*glCompareOperation = GL_ALWAYS;
		return true;
	case NEVER_COMPARE_OPERATOR:
		*glCompareOperation = GL_NEVER;
		return true;
	}
}
