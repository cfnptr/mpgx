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

// TODO: OpenGL renderbuffer optimization

#pragma once
#include "mpgx/_source/image.h"
#include "mpgx/_source/pipeline.h"

typedef struct _VkFramebuffer
{
	Window window;
	Vec2U size;
	Image* colorAttachments;
	uint8_t colorAttachmentCount;
	Image depthStencilAttachment;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
#if MPGX_SUPPORT_VULKAN
	VkRenderPass renderPass;
	VkImageView* imageViews;
	uint8_t attachmentCount;
	VkFramebuffer handle;
#endif
} _VkFramebuffer;
typedef struct _GlFramebuffer
{
	Window window;
	Vec2U size;
	Image* colorAttachments;
	uint8_t colorAttachmentCount;
	Image depthStencilAttachment;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	GLuint handle;
} _GlFramebuffer;
union Framebuffer
{
	_VkFramebuffer vk;
	_GlFramebuffer gl;
};

#if MPGX_SUPPORT_VULKAN
inline static VkRenderPass createVkShadowRenderPass(
	VkDevice device,
	VkFormat format)
{
	VkAttachmentDescription attachmentDescription = {
		0,
		format,
		VK_SAMPLE_COUNT_1_BIT,
		VK_ATTACHMENT_LOAD_OP_CLEAR,
		VK_ATTACHMENT_STORE_OP_STORE,
		VK_ATTACHMENT_LOAD_OP_DONT_CARE,
		VK_ATTACHMENT_STORE_OP_DONT_CARE,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
	};
	VkAttachmentReference attachmentReference = {
		0,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription subpassDescription = {
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		NULL,
		0,
		NULL,
		NULL,
		&attachmentReference,
		0,
		NULL,
	};

	VkSubpassDependency subpassDependencies[2] = {
		{
			VK_SUBPASS_EXTERNAL,
			0,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
		{
			0,
			VK_SUBPASS_EXTERNAL,
			VK_PIPELINE_STAGE_LATE_FRAGMENT_TESTS_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
			VK_ACCESS_SHADER_READ_BIT,
			VK_DEPENDENCY_BY_REGION_BIT,
		},
	};

	VkRenderPassCreateInfo renderPassCreateInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		1,
		&attachmentDescription,
		1,
		&subpassDescription,
		2,
		subpassDependencies,
	};

	VkRenderPass renderPass;

	VkResult vkResult = vkCreateRenderPass(
		device,
		&renderPassCreateInfo,
		NULL,
		&renderPass);

	if (vkResult != VK_SUCCESS)
		return NULL;

	return renderPass;
}
inline static VkImageView createVkImageView(
	VkDevice device,
	VkImage image,
	VkFormat format,
	VkImageAspectFlags aspect)
{
	VkImageViewCreateInfo imageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		{
			aspect,
			0,
			1,
			0,
			1,
		},
	};

	VkImageView imageView;

	VkResult vkResult = vkCreateImageView(
		device,
		&imageViewCreateInfo,
		NULL,
		&imageView);

	if (vkResult != VK_SUCCESS)
		return NULL;

	return imageView;
}
inline static Framebuffer createVkFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	Window window,
	Vec2U size,
	Image* _colorAttachments,
	uint8_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
{
	assert(colorAttachmentCount < UINT8_MAX);

	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	Image* colorAttachments;

	if (colorAttachmentCount != 0)
	{
		colorAttachments = malloc(
			sizeof(Image) * colorAttachmentCount);

		if (colorAttachments == NULL)
		{
			free(framebuffer);
			return NULL;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			assert(
				_colorAttachments[i]->vk.size.x == size.x &&
				_colorAttachments[i]->vk.size.y == size.y);
			assert(_colorAttachments[i]->vk.window == window);
			colorAttachments[i] = _colorAttachments[i];
		}
	}
	else
	{
		colorAttachments = NULL;
	}

	uint8_t attachmentCount = colorAttachmentCount;

	if (depthStencilAttachment != NULL)
		attachmentCount++;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (imageViews == NULL)
	{
		free(colorAttachments);
		free(framebuffer);
		return NULL;
	}

	for (uint8_t i = 0; i < colorAttachmentCount; i++)
	{
		Image attachment = colorAttachments[i];

		VkImageView imageView = createVkImageView(
			device,
			attachment->vk.handle,
			attachment->vk.vkFormat,
			attachment->vk.vkAspect);

		if (imageView == NULL)
		{
			for (uint8_t j = 0; j < i; j++)
			{
				vkDestroyImageView(
					device,
					imageViews[j],
					NULL);
			}

			free(imageViews);
			free(colorAttachments);
			free(framebuffer);
			return NULL;
		}

		imageViews[i] = imageView;
	}

	if (depthStencilAttachment != NULL)
	{
		VkImageView imageView = createVkImageView(
			device,
			depthStencilAttachment->vk.handle,
			depthStencilAttachment->vk.vkFormat,
			depthStencilAttachment->vk.vkAspect);

		if (imageView == NULL)
		{
			for (uint8_t i = 0; i < colorAttachmentCount; i++)
			{
				vkDestroyImageView(
					device,
					imageViews[i],
					NULL);
			}

			free(imageViews);
			free(colorAttachments);
			free(framebuffer);
			return NULL;
		}

		imageViews[colorAttachmentCount] = imageView;
	}

	VkFramebufferCreateInfo framebufferCreateInfo = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		NULL,
		0,
		renderPass,
		attachmentCount,
		imageViews,
		size.x,
		size.y,
		1,
	};

	VkFramebuffer handle;

	VkResult result = vkCreateFramebuffer(
		device,
		&framebufferCreateInfo,
		NULL,
		&handle);

	if (result != VK_SUCCESS)
	{
		for (uint8_t i = 0; i < attachmentCount; i++)
		{
			vkDestroyImageView(
				device,
				imageViews[i],
				NULL);
		}

		free(imageViews);
		free(colorAttachments);
		free(framebuffer);
		return NULL;
	}

	Pipeline* pipelines = malloc(
		sizeof(Pipeline) * pipelineCapacity);

	if (pipelines == NULL)
	{
		vkDestroyFramebuffer(
			device,
			handle,
			NULL);

		for (uint8_t i = 0; i < attachmentCount; i++)
		{
			vkDestroyImageView(
				device,
				imageViews[i],
				NULL);
		}

		free(imageViews);
		free(colorAttachments);
		free(framebuffer);
		return NULL;
	}

	framebuffer->vk.window = window;
	framebuffer->vk.size = size;
	framebuffer->vk.colorAttachments = colorAttachments;
	framebuffer->vk.colorAttachmentCount = colorAttachmentCount;
	framebuffer->vk.depthStencilAttachment = depthStencilAttachment;
	framebuffer->vk.pipelines = pipelines;
	framebuffer->vk.pipelineCapacity = pipelineCapacity;
	framebuffer->vk.pipelineCount = 0;
	framebuffer->vk.renderPass = renderPass;
	framebuffer->vk.imageViews = imageViews;
	framebuffer->vk.attachmentCount = attachmentCount;
	framebuffer->vk.handle = handle;
	return framebuffer;
}
inline static void destroyVkFramebuffer(
	VkDevice device,
	Framebuffer framebuffer)
{
	size_t pipelineCount = framebuffer->vk.pipelineCount;
	Pipeline* pipelines = framebuffer->vk.pipelines;
	Window window = framebuffer->vk.window;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		if (pipeline->vk.onHandleDestroy != NULL)
		{
			pipeline->vk.onHandleDestroy(
				window,
				pipeline->vk.handle);
		}

		destroyVkPipeline(device, pipeline);
	}

	free(pipelines);

	vkDestroyFramebuffer(
		device,
		framebuffer->vk.handle,
		NULL);

	uint8_t attachmentCount = framebuffer->vk.attachmentCount;
	VkImageView* imageViews = framebuffer->vk.imageViews;

	for (uint8_t i = 0; i < attachmentCount; i++)
	{
		vkDestroyImageView(
			device,
			imageViews[i],
			NULL);
	}

	free(imageViews);

	vkDestroyRenderPass(
		device,
		framebuffer->vk.renderPass,
		NULL);

	free(framebuffer->vk.colorAttachments);
	free(framebuffer);
}
inline static void beginVkFramebufferRender(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	Vec2U size,
	Vec4F clearColor,
	float clearDepth,
	uint32_t clearStencil)
{
	VkClearValue clearValues[2];
	clearValues[0].color.float32[0] = clearColor.x;
	clearValues[0].color.float32[1] = clearColor.y;
	clearValues[0].color.float32[2] = clearColor.z;
	clearValues[0].color.float32[3] = clearColor.w;
	clearValues[1].depthStencil.depth = clearDepth;
	clearValues[1].depthStencil.stencil = clearStencil;

	VkRenderPassBeginInfo renderPassBeginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		renderPass,
		framebuffer,
		{
			0, 0, size.x, size.y,
		},
		2,
		clearValues,
	};

	vkCmdBeginRenderPass(
		commandBuffer,
		&renderPassBeginInfo,
		VK_SUBPASS_CONTENTS_INLINE);
}
inline static void endVkFramebufferRender(
	VkCommandBuffer commandBuffer)
{
	vkCmdEndRenderPass(commandBuffer);
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
	Vec2U size,
	Image* _colorAttachments,
	uint8_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
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

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			assert(
				_colorAttachments[i]->gl.size.x == size.x &&
				_colorAttachments[i]->gl.size.y == size.y);
			assert(_colorAttachments[i]->gl.window == window);

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
					GL_COLOR_ATTACHMENT0 + (GLenum)i,
					colorAttachment->gl.glType,
					colorAttachment->gl.handle,
					GL_ZERO);
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

	Pipeline* pipelines = malloc(
		sizeof(Pipeline) * pipelineCapacity);

	if (pipelines == NULL)
	{
		free(colorAttachments);
		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(framebuffer);
		return NULL;
	}

	framebuffer->gl.window = window;
	framebuffer->gl.size = size;
	framebuffer->gl.colorAttachments = colorAttachments;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;
	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;
	framebuffer->gl.pipelines = pipelines;
	framebuffer->gl.pipelineCapacity = 0;
	framebuffer->gl.pipelineCount = 0;
	framebuffer->gl.handle = handle;
	return framebuffer;
}
inline static void destroyGlFramebuffer(Framebuffer framebuffer)
{
	size_t pipelineCount = framebuffer->gl.pipelineCount;
	Pipeline* pipelines = framebuffer->gl.pipelines;
	Window window = framebuffer->vk.window;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		if (pipeline->vk.onHandleDestroy != NULL)
		{
			pipeline->vk.onHandleDestroy(
				window,
				pipeline->vk.handle);
		}

		destroyGlPipeline(pipeline);
	}

	free(pipelines);

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
