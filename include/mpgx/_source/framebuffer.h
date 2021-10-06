// Copyright 2020-2021 Nikita Fediuchin. All rights reserved.
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once
#include "mpgx/_source/image.h"

typedef struct _VkFramebuffer
{
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
#if MPGX_SUPPORT_VULKAN
	VkFramebuffer handle;
#endif
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
	VkDevice device,
	Window window,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	// TODO:
	VkRenderPassCreateInfo renderPassCreateInfo;

	VkRenderPass renderPass;

	VkFramebufferCreateInfo framebufferCreateInfo = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		NULL,
		0,

	};

	VkFramebuffer handle;

	VkResult result = vkCreateFramebuffer(
		device,
		&framebufferCreateInfo,
		NULL,
		&handle);

	if (result != VK_SUCCESS)
	{
		free(framebuffer);
		return NULL;
	}

	framebuffer->vk.handle = handle;
	return framebuffer;
}
inline static void destroyVkFramebuffer(
	VkDevice device,
	Framebuffer framebuffer)
{
	vkDestroyFramebuffer(
		device,
		framebuffer->vk.handle,
		NULL);
	free(framebuffer);
}
inline static void beginVkFramebufferRender(
	Framebuffer framebuffer)
{
	// TODO:
}
inline static void endVkFramebufferRender(
	Window window)
{
	// TODO:
}
inline static void clearVkFramebuffer(
	bool clearColorBuffer,
	bool clearDepthBuffer,
	bool clearStencilBuffer,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	// TODO:
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
			ImageFormat format = colorAttachment->gl.format;

			switch (format)
			{
			default:
				free(colorAttachments);
				glDeleteFramebuffers(
					GL_ONE,
					&handle);
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
		ImageFormat format = depthStencilAttachment->gl.format;

		switch (format)
		{
		default:
			free(colorAttachments);
			glDeleteFramebuffers(
				GL_ONE,
				&handle);
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

	GLenum status = glCheckFramebufferStatus(GL_FRAMEBUFFER);

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

		free(colorAttachments);
		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(framebuffer);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		free(colorAttachments);
		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(framebuffer);
		return NULL;
	}

	framebuffer->gl.window = window;
	framebuffer->gl.handle = handle;
	framebuffer->gl.colorAttachments = colorAttachments;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;
	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;
	return framebuffer;
}
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
inline static void beginGlFramebufferRender(
	GLuint framebuffer)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		framebuffer);
	assertOpenGL();
}
inline static void endGlFramebufferRender()
{
	/*glBindFramebuffer(
		GL_FRAMEBUFFER,
		GL_ZERO);*/
	assertOpenGL();
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
