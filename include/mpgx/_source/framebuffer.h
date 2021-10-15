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

typedef struct _BaseFramebuffer
{
	Window window;
	Vec2U size;
	Image* attachments;
	uint8_t attachmentCount;
	bool isDefault;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
} _BaseFramebuffer;
typedef struct _VkFramebuffer
{
	Window window;
	Vec2U size;
	Image* attachments;
	uint8_t attachmentCount;
	bool isDefault;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
#if MPGX_SUPPORT_VULKAN
	VkRenderPass renderPass;
	VkImageView* imageViews;
	VkFramebuffer handle;
#endif
} _VkFramebuffer;
typedef struct _GlFramebuffer
{
	Window window;
	Vec2U size;
	Image* attachments;
	uint8_t attachmentCount;
	bool isDefault;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	GLuint handle;
} _GlFramebuffer;
union Framebuffer
{
	_BaseFramebuffer base;
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
inline static Framebuffer createDefaultVkFramebuffer(
	VkRenderPass renderPass,
	VkImageView* _imageViews,
	VkFramebuffer handle,
	Window window,
	Vec2U size,
	size_t attachmentCount,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (imageViews == NULL)
	{
		free(framebuffer);
		return NULL;
	}

	for (size_t i = 0; i < attachmentCount; i++)
		imageViews[i] = _imageViews[i];

	Pipeline* pipelines = malloc(
		pipelineCapacity * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		free(imageViews);
		free(framebuffer);
		return NULL;
	}

	framebuffer->vk.window = window;
	framebuffer->vk.size = size;
	framebuffer->vk.attachments = NULL;
	framebuffer->vk.attachmentCount = attachmentCount;
	framebuffer->vk.isDefault = true;
	framebuffer->vk.pipelines = pipelines;
	framebuffer->vk.pipelineCapacity = pipelineCapacity;
	framebuffer->vk.pipelineCount = 0;
	framebuffer->vk.renderPass = renderPass;
	framebuffer->vk.imageViews = imageViews;
	framebuffer->vk.handle = handle;
	return framebuffer;
}
inline static Framebuffer createVkFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	Window window,
	Vec2U size,
	Image* _attachments,
	size_t attachmentCount,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	Image* attachments = malloc(
		attachmentCount * sizeof(Image));

	if (attachments == NULL)
	{
		free(framebuffer);
		return NULL;
	}

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (imageViews == NULL)
	{
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	for (size_t i = 0; i < attachmentCount; i++)
	{
		Image attachment = _attachments[i];

		assert(
			attachment->vk.size.x == size.x &&
			attachment->vk.size.y == size.y);
		assert(attachment->vk.window == window);

		VkImageView imageView = createVkImageView(
			device,
			attachment->vk.handle,
			attachment->vk.vkFormat,
			attachment->vk.vkAspect);

		if (imageView == NULL)
		{
			for (size_t j = 0; j < i; j++)
			{
				vkDestroyImageView(
					device,
					imageViews[j],
					NULL);
			}

			free(imageViews);
			free(attachments);
			free(framebuffer);
			return NULL;
		}

		attachments[i] = attachment;
		imageViews[i] = imageView;
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
		for (size_t i = 0; i < attachmentCount; i++)
		{
			vkDestroyImageView(
				device,
				imageViews[i],
				NULL);
		}

		free(imageViews);
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	Pipeline* pipelines = malloc(
		pipelineCapacity * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		vkDestroyFramebuffer(
			device,
			handle,
			NULL);

		for (size_t i = 0; i < attachmentCount; i++)
		{
			vkDestroyImageView(
				device,
				imageViews[i],
				NULL);
		}

		free(imageViews);
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	framebuffer->vk.window = window;
	framebuffer->vk.size = size;
	framebuffer->vk.attachments = attachments;
	framebuffer->vk.attachmentCount = attachmentCount;
	framebuffer->vk.isDefault = false;
	framebuffer->vk.pipelines = pipelines;
	framebuffer->vk.pipelineCapacity = pipelineCapacity;
	framebuffer->vk.pipelineCount = 0;
	framebuffer->vk.renderPass = renderPass;
	framebuffer->vk.imageViews = imageViews;
	framebuffer->vk.handle = handle;
	return framebuffer;
}

inline static void destroyVkPipeline(
	VkDevice device, Pipeline pipeline);

inline static void destroyVkFramebuffer(
	VkDevice device,
	Framebuffer framebuffer)
{
	size_t pipelineCount = framebuffer->vk.pipelineCount;
	Pipeline* pipelines = framebuffer->vk.pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		OnPipelineHandleDestroy onDestroy =
			getPipelineOnHandleDestroy(pipeline);

		if (onDestroy != NULL)
			onDestroy(getPipelineHandle(pipeline));

		destroyVkPipeline(device, pipeline);
	}

	free(pipelines);

	size_t attachmentCount = framebuffer->vk.attachmentCount;
	VkImageView* imageViews = framebuffer->vk.imageViews;

	if (framebuffer->vk.isDefault == false)
	{
		vkDestroyFramebuffer(
			device,
			framebuffer->vk.handle,
			NULL);

		for (size_t i = 0; i < attachmentCount; i++)
		{
			vkDestroyImageView(
				device,
				imageViews[i],
				NULL);
		}

		free(framebuffer->vk.attachments);

		vkDestroyRenderPass(
			device,
			framebuffer->vk.renderPass,
			NULL);
	}

	free(imageViews);
	free(framebuffer);
}
inline static void beginVkFramebufferRender(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	Vec2U size,
	bool clearColor,
	bool clearDepth,
	bool clearStencil,
	Vec4F colorValue,
	float depthValue,
	uint32_t stencilValue)
{
	VkClearValue clearValues[2];
	size_t clearValueCount = 0;

	if (clearColor == true)
	{
		VkClearValue clearValue;
		clearValue.color.float32[0] = colorValue.x;
		clearValue.color.float32[1] = colorValue.y;
		clearValue.color.float32[2] = colorValue.z;
		clearValue.color.float32[3] = colorValue.w;
		clearValues[clearValueCount++] = clearValue;
	}
	if (clearDepth == true || clearStencil == true)
	{
		VkClearValue clearValue;
		clearValue.depthStencil.depth = depthValue;
		clearValue.depthStencil.stencil = stencilValue;
		clearValues[clearValueCount++] = clearValue;
	}

	VkRenderPassBeginInfo renderPassBeginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		renderPass,
		framebuffer,
		{
			0, 0, size.x, size.y,
		},
		clearValueCount,
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
	VkCommandBuffer commandBuffer,
	Vec2U framebufferSize,
	bool clearColor,
	bool clearDepth,
	bool clearStencil,
	Vec4F colorValue,
	float depthValue,
	uint32_t stencilValue)
{
	VkClearAttachment clearAttachments[3];
	size_t attachmentCount = 0;

	if (clearColor == true)
	{
		VkClearValue clearValue;
		clearValue.color.float32[0] = colorValue.x;
		clearValue.color.float32[1] = colorValue.y;
		clearValue.color.float32[2] = colorValue.z;
		clearValue.color.float32[3] = colorValue.w;

		VkClearAttachment clearAttachment = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			clearValue,
		};

		clearAttachments[attachmentCount++] = clearAttachment;
	}
	if (clearDepth == true)
	{
		VkClearValue clearValue;
		clearValue.depthStencil.depth = depthValue;

		VkClearAttachment clearAttachment = {
			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			clearValue,
		};

		clearAttachments[attachmentCount++] = clearAttachment;
	}
	if (clearStencil == true)
	{
		VkClearValue clearValue;
		clearValue.depthStencil.stencil = stencilValue;

		VkClearAttachment clearAttachment = {
			VK_IMAGE_ASPECT_STENCIL_BIT,
			0,
			clearValue,
		};

		clearAttachments[attachmentCount++] = clearAttachment;
	}

	VkClearRect clearRect = {
		{
			0, 0,
			framebufferSize.x,
			framebufferSize.y,
		},
		0,
		1,
	};

	vkCmdClearAttachments(
		commandBuffer,
		attachmentCount,
		clearAttachments,
		1,
		&clearRect);
}
#endif

inline static Framebuffer createDefaultGlFramebuffer(
	Window window,
	Vec2U size,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	Pipeline* pipelines = malloc(
		sizeof(Pipeline) * pipelineCapacity);

	if (pipelines == NULL)
	{
		free(framebuffer);
		return NULL;
	}

	framebuffer->gl.window = window;
	framebuffer->gl.size = size;
	framebuffer->gl.attachments = NULL;
	framebuffer->gl.attachmentCount = 0;
	framebuffer->gl.isDefault = true;
	framebuffer->gl.pipelines = pipelines;
	framebuffer->gl.pipelineCapacity = pipelineCapacity;
	framebuffer->gl.pipelineCount = 0;
	framebuffer->gl.handle = GL_ZERO;
	return framebuffer;
}
inline static Framebuffer createGlFramebuffer(
	Window window,
	Vec2U size,
	Image* _attachments,
	size_t attachmentCount,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = malloc(
		sizeof(union Framebuffer));

	if (framebuffer == NULL)
		return NULL;

	Image* attachments = malloc(
		attachmentCount * sizeof(Image));

	if (attachments == NULL)
	{
		free(framebuffer);
		return NULL;
	}

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenFramebuffers(
		GL_ONE,
		&handle);
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		handle);

	size_t colorAttachmentCount = 0;

	for (size_t i = 0; i < attachmentCount; i++)
	{
		Image attachment = _attachments[i];

		assert(
			attachment->gl.size.x == size.x &&
			attachment->gl.size.y == size.y);
		assert(attachment->gl.window == window);

		ImageFormat format = attachment->gl.format;

		switch (format)
		{
		default:
			glDeleteFramebuffers(
				GL_ONE,
				&handle);
			free(attachments);
			free(framebuffer);
			return NULL;
		case R8G8B8A8_UNORM_IMAGE_FORMAT:
		case R8G8B8A8_SRGB_IMAGE_FORMAT:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_COLOR_ATTACHMENT0 + (GLenum)colorAttachmentCount,
				attachment->gl.glType,
				attachment->gl.handle,
				GL_ZERO);
			colorAttachmentCount++;
			break;
		case D16_UNORM_IMAGE_FORMAT:
		case D32_SFLOAT_IMAGE_FORMAT:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_ATTACHMENT,
				attachment->gl.glType,
				attachment->gl.handle,
				GL_ZERO);
			break;
		case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
			glFramebufferTexture2D(
				GL_FRAMEBUFFER,
				GL_DEPTH_STENCIL_ATTACHMENT,
				attachment->gl.glType,
				attachment->gl.handle,
				GL_ZERO);
			break;
		}

		attachments[i] = attachment;
	}

	if (colorAttachmentCount == 0)
	{
		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
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

		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	Pipeline* pipelines = malloc(
		pipelineCapacity * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		glDeleteFramebuffers(
			GL_ONE,
			&handle);
		free(attachments);
		free(framebuffer);
		return NULL;
	}

	framebuffer->gl.window = window;
	framebuffer->gl.size = size;
	framebuffer->gl.attachments = attachments;
	framebuffer->gl.attachmentCount = attachmentCount;
	framebuffer->gl.isDefault = false;
	framebuffer->gl.pipelines = pipelines;
	framebuffer->gl.pipelineCapacity = pipelineCapacity;
	framebuffer->gl.pipelineCount = 0;
	framebuffer->gl.handle = handle;
	return framebuffer;
}

inline static void destroyGlPipeline(Pipeline pipeline);

inline static void destroyGlFramebuffer(
	Framebuffer framebuffer)
{
	size_t pipelineCount = framebuffer->gl.pipelineCount;
	Pipeline* pipelines = framebuffer->gl.pipelines;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		OnPipelineHandleDestroy onDestroy =
			getPipelineOnHandleDestroy(pipeline);

		if (onDestroy != NULL)
			onDestroy(getPipelineHandle(pipeline));

		destroyGlPipeline(pipeline);
	}

	free(pipelines);

	if (framebuffer->gl.isDefault == false)
	{
		makeWindowContextCurrent(
			framebuffer->gl.window);

		glDeleteFramebuffers(
			GL_ONE,
			&framebuffer->gl.handle);
		assertOpenGL();

		free(framebuffer->gl.attachments);
	}

	free(framebuffer);
}
inline static void beginGlFramebufferRender(
	GLuint framebuffer,
	bool clearColor,
	bool clearDepth,
	bool clearStencil,
	Vec4F colorValue,
	float depthValue,
	uint32_t stencilValue)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		framebuffer);

	GLbitfield clearMask = 0;

	if (clearColor == true)
	{
		glClearColor(
			colorValue.x,
			colorValue.y,
			colorValue.z,
			colorValue.w);
		glColorMask(
			GL_TRUE, GL_TRUE,
			GL_TRUE, GL_TRUE);
		clearMask |= GL_COLOR_BUFFER_BIT;
	}
	if (clearDepth == true)
	{
		glClearDepth(depthValue);
		glDepthMask(GL_TRUE);
		clearMask |= GL_DEPTH_BUFFER_BIT;
	}
	if (clearStencil == true)
	{
		glClearStencil((GLint)stencilValue);
		glStencilMask(UINT32_MAX);
		clearMask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(clearMask);
	assertOpenGL();
}
inline static void endGlFramebufferRender()
{
	assertOpenGL();
}
inline static void clearGlFramebuffer(
	bool clearColor,
	bool clearDepth,
	bool clearStencil,
	Vec4F colorValue,
	float depthValue,
	uint32_t stencilValue)
{
	GLbitfield clearMask = 0;

	if (clearColor == true)
	{
		glClearColor(
			colorValue.x,
			colorValue.y,
			colorValue.z,
			colorValue.w);
		glColorMask(
			GL_TRUE, GL_TRUE,
			GL_TRUE, GL_TRUE);
		clearMask |= GL_COLOR_BUFFER_BIT;
	}
	if (clearDepth == true)
	{
		glClearDepth(depthValue);
		glDepthMask(GL_TRUE);
		clearMask |= GL_DEPTH_BUFFER_BIT;
	}
	if (clearStencil == true)
	{
		glClearStencil((GLint)stencilValue);
		glStencilMask(UINT32_MAX);
		clearMask |= GL_STENCIL_BUFFER_BIT;
	}

	glClear(clearMask);
	assertOpenGL();
}
