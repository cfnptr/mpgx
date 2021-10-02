#pragma once
#include "mpgx/window.h"
#include "mpgx/_source/graphics.h"

#include <stdio.h>
#include <stdbool.h>

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

inline static bool getGlCompareOperator(
	uint8_t compareOperator,
	GLenum* glCompareOperator)
{
	switch (compareOperator)
	{
	default:
		return false;
	case LESS_COMPARE_OPERATOR:
		*glCompareOperator = GL_LESS;
		return true;
	case LESS_OR_EQUAL_COMPARE_OPERATOR:
		*glCompareOperator = GL_LEQUAL;
		return true;
	case GREATER_OR_EQUAL_COMPARE_OPERATOR:
		*glCompareOperator = GL_GEQUAL;
		return true;
	case GREATER_COMPARE_OPERATOR:
		*glCompareOperator = GL_GREATER;
		return true;
	case EQUAL_COMPARE_OPERATOR:
		*glCompareOperator = GL_EQUAL;
		return true;
	case NOT_EQUAL_COMPARE_OPERATOR:
		*glCompareOperator = GL_NOTEQUAL;
		return true;
	case ALWAYS_COMPARE_OPERATOR:
		*glCompareOperator = GL_ALWAYS;
		return true;
	case NEVER_COMPARE_OPERATOR:
		*glCompareOperator = GL_NEVER;
		return true;
	}
}
