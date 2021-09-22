#pragma once
#include "mpgx/_source/image.h"

typedef struct _VkFramebuffer
{
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
} _VkFramebuffer;
typedef struct _GlFramebuffer
{
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	GLuint handle;
} _GlFramebuffer;
union Framebuffer
{
	_VkFramebuffer vk;
	_GlFramebuffer gl;
};

#if MPGX_SUPPORT_VULKAN
inline static Framebuffer createVkFramebuffer(
	Window window,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	// TODO:
	abort();
}
#endif

inline static Framebuffer createGlFramebuffer(
	Window window,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenFramebuffers(
		GL_ONE,
		&handle);
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		handle);

	Image* colorAttachments;

	if (_colorAttachments != NULL)
	{
		colorAttachments = malloc(
			sizeof(Image) * colorAttachmentCount);

		if (colorAttachments == NULL)
		{
			glDeleteFramebuffers(
				GL_ONE,
				&handle);
			free(framebuffer);
			return NULL;
		}

		GLenum colorIndex = 0;

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = _colorAttachments[i];
			uint8_t format = colorAttachment->gl.format;

			switch (format)
			{
			default:
				glDeleteFramebuffers(
					GL_ONE,
					&handle);
				free(colorAttachments);
				free(framebuffer);
				return NULL;
			case R8G8B8A8_UNORM_IMAGE_FORMAT:
			case R8G8B8A8_SRGB_IMAGE_FORMAT:
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					GL_COLOR_ATTACHMENT0 + colorIndex,
					colorAttachment->gl.glType,
					colorAttachment->gl.handle,
					GL_ZERO);
				colorIndex++;
				break;
			}

			colorAttachments[i] = colorAttachment;
		}
	}
	else
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
		colorAttachments = NULL;
	}

	if (depthStencilAttachment != NULL)
	{
		uint8_t format = depthStencilAttachment->gl.format;

		switch (format)
		{
		default:
			glDeleteFramebuffers(
				GL_ONE,
				&handle);
			free(colorAttachments);
			free(framebuffer);
			return NULL;
		case D16_UNORM_IMAGE_FORMAT:
		case D32_SFLOAT_IMAGE_FORMAT:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				depthStencilAttachment->gl.glType,
				depthStencilAttachment->gl.handle,
				GL_ZERO);
			break;
		case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_STENCIL_ATTACHMENT,
				depthStencilAttachment->gl.glType,
				depthStencilAttachment->gl.handle,
				GL_ZERO);
			break;
		}
	}

	GLenum status = glCheckFramebufferStatus(
		GL_FRAMEBUFFER);

	if (status != GL_FRAMEBUFFER_COMPLETE)
	{
#ifndef NDEBUG
		const char* statusName;

		switch (status)
		{
		default:
			statusName = "UNKNOWN_STATUS";
			break;
		case GL_FRAMEBUFFER_UNDEFINED:
			statusName = "GL_FRAMEBUFFER_UNDEFINED";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT:
			statusName = "GL_FRAMEBUFFER_INCOMPLETE_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT:
			statusName = "GL_FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT";
			break;
		case GL_FRAMEBUFFER_UNSUPPORTED:
			statusName = "GL_FRAMEBUFFER_UNSUPPORTED";
			break;
		case GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE:
			statusName = "GL_FRAMEBUFFER_INCOMPLETE_MULTISAMPLE";
			break;
		}

		fprintf(stderr,
			"OpenGL framebuffer create error: %s\n",
			statusName);
#endif

		assertOpenGL();

		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(colorAttachments);
		free(framebuffer);
		return NULL;
	}

	assertOpenGL();

	framebuffer->gl.window = window;
	framebuffer->gl.handle = handle;
	framebuffer->gl.colorAttachments = colorAttachments;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;
	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;
	return framebuffer;
}

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkFramebuffer(Framebuffer framebuffer)
{
	// TODO:
}
#endif

inline static void destroyGlFramebuffer(Framebuffer framebuffer)
{
	makeWindowContextCurrent(
		framebuffer->gl.window);

	glDeleteFramebuffers(
		GL_ONE,
		&framebuffer->gl.handle);
	assertOpenGL();

	free(framebuffer->gl.colorAttachments);
	free(framebuffer);
}

inline static void beginVkFramebufferRender(
	Framebuffer framebuffer)
{
	// TODO:
}
inline static void beginGlFramebufferRender(
	GLuint framebuffer)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		framebuffer);
	assertOpenGL();
}

inline static void endVkFramebufferRender(
	Window window)
{
	// TODO:
}
inline static void endGlFramebufferRender()
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		GL_ZERO);
	assertOpenGL();
}

inline static void clearVkFramebuffer(
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{

}
inline static void clearGlFramebuffer(
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	GLbitfield clearMask = 0;

	if (clearColorBuffer == true)
	{
		glClearColor(
			clearColor.x,
			clearColor.y,
			clearColor.z,
			clearColor.w);
		glColorMask(
			GL_TRUE, GL_TRUE,
			GL_TRUE, GL_TRUE);
		clearMask |= GL_COLOR_BUFFER_BIT;
	}
	if (clearDepthBuffer == true)
	{
		glClearDepth(clearDepth);
		glDepthMask(GL_TRUE);
		clearMask |= GL_DEPTH_BUFFER_BIT;
	}
	if (clearStencilBuffer == true)
	{
		glClearStencil((GLint)clearStencil);
		glStencilMask(UINT32_MAX);
		clearMask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(clearMask);
	assertOpenGL();
}
