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

typedef struct BaseFramebuffer_T
{
	bool isDefault;
	Window window;
	Vec2U size;
	bool useBeginClear;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
} BaseFramebuffer_T;
typedef struct VkFramebuffer_T
{
	bool isDefault;
	Window window;
	Vec2U size;
	bool useBeginClear;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
#if MPGX_SUPPORT_VULKAN
	VkRenderPass renderPass;
	VkFramebuffer handle;
#endif
} VkFramebuffer_T;
typedef struct GlFramebuffer_T
{
	bool isDefault;
	Window window;
	Vec2U size;
	bool useBeginClear;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	Pipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	GLuint handle;
} GlFramebuffer_T;
union Framebuffer_T
{
	BaseFramebuffer_T base;
	VkFramebuffer_T vk;
	GlFramebuffer_T gl;
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkPipelineCreateInfo
{
	uint32_t vertexBindingDescriptionCount;
	const VkVertexInputBindingDescription* vertexBindingDescriptions;
	uint32_t vertexAttributeDescriptionCount;
	const VkVertexInputAttributeDescription* vertexAttributeDescriptions;
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pushConstantRanges;
} VkPipelineCreateInfo;

inline static VkRenderPass createVkGeneralRenderPass(
	VkDevice device,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	size_t attachmentCount = depthStencilAttachment != NULL ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkAttachmentDescription* attachmentDescriptions = malloc(
		attachmentCount * sizeof(VkAttachmentDescription));

	if (attachmentDescriptions == NULL)
		return NULL;

	VkAttachmentReference* colorReferences;

	if (colorAttachmentCount != 0)
	{
		colorReferences = malloc(
			colorAttachmentCount * sizeof(VkAttachmentDescription));

		if (colorReferences == NULL)
		{
			free(attachmentDescriptions);
			return NULL;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = colorAttachments[i];

			VkAttachmentDescription attachmentDescription = {
				0,
				colorAttachment->vk.vkFormat,
				VK_SAMPLE_COUNT_1_BIT,
				useBeginClear == true ?
					VK_ATTACHMENT_LOAD_OP_CLEAR :
					VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_STORE,
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
				VK_ATTACHMENT_STORE_OP_DONT_CARE,
				VK_IMAGE_LAYOUT_UNDEFINED,
				VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			};
			VkAttachmentReference attachmentReference = {
				(uint32_t)i,
				VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
			};

			attachmentDescriptions[i] = attachmentDescription;
			colorReferences[i] = attachmentReference;
		}
	}
	else
	{
		colorReferences = NULL;
	}

	if (depthStencilAttachment != NULL)
	{
		ImageFormat format = depthStencilAttachment->vk.format;

		VkAttachmentLoadOp stencilLoadOp;
		VkAttachmentStoreOp stencilStoreOp;

		switch (format)
		{
		default:
			abort();
		case D16_UNORM_IMAGE_FORMAT:
		case D32_SFLOAT_IMAGE_FORMAT:
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		case D16_UNORM_S8_UINT_IMAGE_FORMAT:
		case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
			stencilLoadOp = useBeginClear == true ?
				VK_ATTACHMENT_LOAD_OP_CLEAR :
				VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			break;
		}

		VkAttachmentDescription attachmentDescription = {
			0,
			depthStencilAttachment->vk.vkFormat,
			VK_SAMPLE_COUNT_1_BIT,
			useBeginClear == true ?
				VK_ATTACHMENT_LOAD_OP_CLEAR :
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_STORE,
			stencilLoadOp,
			stencilStoreOp,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_READ_ONLY_OPTIMAL,
		};

		attachmentDescriptions[colorAttachmentCount] = attachmentDescription;
	}

	VkAttachmentReference depthStencilAttachmentReference = {
		(uint32_t)colorAttachmentCount,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};
	VkSubpassDescription subpassDescription = {
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		NULL,
		(uint32_t)colorAttachmentCount,
		colorReferences,
		NULL,
		depthStencilAttachment != NULL ?
			&depthStencilAttachmentReference : NULL,
		0,
		NULL,
	};

	VkRenderPassCreateInfo renderPassCreateInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		(uint32_t)attachmentCount,
		attachmentDescriptions,
		1,
		&subpassDescription,
		0,
		NULL,
	};

	VkRenderPass renderPass;

	VkResult vkResult = vkCreateRenderPass(
		device,
		&renderPassCreateInfo,
		NULL,
		&renderPass);

	free(colorReferences);
	free(attachmentDescriptions);

	if (vkResult != VK_SUCCESS)
		return NULL;

	return renderPass;
}
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

inline static VkFramebuffer createVkFramebufferHandle(
	VkDevice device,
	VkRenderPass renderPass,
	size_t attachmentCount,
	VkImageView* imageViews,
	Vec2U size)
{
	VkFramebufferCreateInfo framebufferCreateInfo = {
		VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
		NULL,
		0,
		renderPass,
		(uint32_t)attachmentCount,
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
		return NULL;

	return handle;
}

inline static void destroyVkFramebuffer(
	VkDevice device,
	Framebuffer framebuffer)
{
	if (framebuffer == NULL)
		return;

	if (framebuffer->vk.isDefault == false)
	{
		vkDestroyFramebuffer(
			device,
			framebuffer->vk.handle,
			NULL);
		vkDestroyRenderPass(
			device,
			framebuffer->vk.renderPass,
			NULL);
		free(framebuffer->vk.colorAttachments);
	}

	assert(framebuffer->vk.pipelineCount == 0);

	free(framebuffer->vk.pipelines);
	free(framebuffer);
}

inline static Framebuffer createDefaultVkFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	VkFramebuffer handle,
	Window window,
	Vec2U size)
{
	Framebuffer framebuffer = calloc(1,
		sizeof(Framebuffer_T));

	if (framebuffer == NULL)
		return NULL;

	framebuffer->vk.isDefault = true;
	framebuffer->vk.window = window;
	framebuffer->vk.size = size;
	framebuffer->vk.useBeginClear = true;
	framebuffer->vk.colorAttachments = NULL;
	framebuffer->vk.colorAttachmentCount = 1;
	framebuffer->vk.depthStencilAttachment = NULL;

	Pipeline* pipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		destroyVkFramebuffer(
			device,
			framebuffer);
		return NULL;
	}

	framebuffer->vk.pipelines = pipelines;
	framebuffer->vk.pipelineCapacity = MPGX_DEFAULT_CAPACITY;
	framebuffer->vk.pipelineCount = 0;
	framebuffer->vk.renderPass = renderPass;
	framebuffer->vk.handle = handle;
	return framebuffer;
}
inline static Framebuffer createVkFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	Window window,
	Vec2U size,
	bool useBeginClear,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = calloc(1,
		sizeof(Framebuffer_T));

	if (framebuffer == NULL)
		return NULL;

	framebuffer->vk.isDefault = false;
	framebuffer->vk.window = window;
	framebuffer->vk.size = size;
	framebuffer->vk.useBeginClear = useBeginClear;

	size_t attachmentCount = depthStencilAttachment != NULL ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (imageViews == NULL)
	{
		destroyVkFramebuffer(
			device,
			framebuffer);
		return NULL;
	}

	Image* colorAttachments;

	if (colorAttachmentCount != 0)
	{
		colorAttachments = malloc(
			colorAttachmentCount * sizeof(Image));

		if (colorAttachments == NULL)
		{
			free(imageViews);
			destroyVkFramebuffer(
				device,
				framebuffer);
			return NULL;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = _colorAttachments[i];

			assert(colorAttachment != NULL);
			assert(
				colorAttachment->vk.size.x == size.x &&
				colorAttachment->vk.size.y == size.y);
			assert(colorAttachment->vk.window == window);

			colorAttachments[i] = colorAttachment;
			imageViews[i] = colorAttachment->vk.imageView;
		}
	}
	else
	{
		colorAttachments = NULL;
	}

	framebuffer->vk.colorAttachments = colorAttachments;
	framebuffer->vk.colorAttachmentCount = colorAttachmentCount;

	if (depthStencilAttachment != NULL)
	{
		imageViews[colorAttachmentCount] =
			depthStencilAttachment->vk.imageView;
	}

	framebuffer->vk.depthStencilAttachment = depthStencilAttachment;

	Pipeline* pipelines = malloc(
		pipelineCapacity * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		destroyVkFramebuffer(
			device,
			framebuffer);
		return NULL;
	}

	framebuffer->vk.pipelines = pipelines;
	framebuffer->vk.pipelineCapacity = pipelineCapacity;
	framebuffer->vk.pipelineCount = 0;

	VkFramebuffer handle = createVkFramebufferHandle(
		device,
		renderPass,
		attachmentCount,
		imageViews,
		size);

	free(imageViews);

	if (handle == NULL)
	{
		destroyVkFramebuffer(
			device,
			framebuffer);
		return NULL;
	}

	framebuffer->vk.renderPass = renderPass;
	framebuffer->vk.handle = handle;
	return framebuffer;
}

inline static bool recreateVkPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	Pipeline pipeline,
	size_t colorAttachmentCount,
	VkPipelineCreateInfo* createInfo);

inline static bool setVkFramebufferAttachments(
	VkDevice device,
	VkRenderPass renderPass,
	Framebuffer framebuffer,
	Vec2U size,
	bool useBeginClear,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	size_t attachmentCount = depthStencilAttachment != NULL ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (imageViews == NULL)
		return false;

	Image* colorAttachments;

	if (colorAttachmentCount != 0)
	{
		colorAttachments = malloc(
			colorAttachmentCount * sizeof(Image));

		if (colorAttachments == NULL)
		{
			free(imageViews);
			return false;
		}

		Window window = framebuffer->vk.window;

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = _colorAttachments[i];

			assert(colorAttachment != NULL);
			assert(colorAttachment->vk.size.x == size.x &&
				colorAttachment->vk.size.y == size.y);
			assert(colorAttachment->vk.window == window);

			colorAttachments[i] = colorAttachment;
			imageViews[i] = colorAttachment->vk.imageView;
		}
	}
	else
	{
		colorAttachments = NULL;
	}

	if (depthStencilAttachment != NULL)
	{
		imageViews[colorAttachmentCount] =
			depthStencilAttachment->vk.imageView;
	}

	VkFramebuffer handle = createVkFramebufferHandle(
		device,
		renderPass,
		attachmentCount,
		imageViews,
		size);

	free(imageViews);

	if (handle == NULL)
	{
		free(colorAttachments);
		return false;
	}

	Pipeline* pipelines = framebuffer->vk.pipelines;
	size_t pipelineCount = framebuffer->vk.pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		OnPipelineHandleResize onResize =
			getPipelineOnHandleResize(pipeline);

		VkPipelineCreateInfo createInfo;

		bool result = onResize(
			pipeline,
			size,
			&createInfo);
		result &= recreateVkPipelineHandle(
			device,
			renderPass,
			pipeline,
			colorAttachmentCount,
			&createInfo);

		if (result == false)
		{
			vkDestroyFramebuffer(
				device,
				handle,
				NULL);
			free(colorAttachments);
			return false;
		}
	}

	vkDestroyFramebuffer(
		device,
		framebuffer->vk.handle,
		NULL);
	vkDestroyRenderPass(
		device,
		framebuffer->vk.renderPass,
		NULL);
	free(framebuffer->vk.colorAttachments);

	framebuffer->vk.size = size;
	framebuffer->vk.useBeginClear = useBeginClear;
	framebuffer->vk.colorAttachments = colorAttachments;
	framebuffer->vk.colorAttachmentCount = colorAttachmentCount;
	framebuffer->vk.depthStencilAttachment = depthStencilAttachment;
	framebuffer->vk.handle = handle;
	framebuffer->vk.renderPass = renderPass;
	return true;
}

inline static void beginVkFramebufferRender(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	Vec2U size,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	VkRenderPassBeginInfo renderPassBeginInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
		NULL,
		renderPass,
		framebuffer,
		{
			0, 0,
			size.x, size.y,
		},
		(uint32_t)clearValueCount,
		(const VkClearValue*)clearValues,
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
	bool hasDepthAttachment,
	bool hasStencilAttachment,
	const bool* _clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	// TODO: move allocation to framebuffer object (cache it)
	VkClearAttachment* clearAttachments = malloc(
		clearValueCount * sizeof(VkClearAttachment));

	if (clearAttachments == NULL)
		abort();

	size_t clearAttachmentCount = 0;

	size_t colorAttachmentCount =
		hasDepthAttachment == true || hasStencilAttachment == true ?
		clearValueCount - 1 : clearValueCount;

	for (size_t i = 0; i < colorAttachmentCount; i++)
	{
		if (_clearAttachments[i] == false)
			continue;

		LinearColor value = clearValues[i].color;

		VkClearValue clearValue;
		clearValue.color.float32[0] = value.r;
		clearValue.color.float32[1] = value.g;
		clearValue.color.float32[2] = value.b;
		clearValue.color.float32[3] = value.a;

		VkClearAttachment clearAttachment = {
			VK_IMAGE_ASPECT_COLOR_BIT,
			(uint32_t)i,
			clearValue,
		};

		clearAttachments[clearAttachmentCount++] = clearAttachment;
	}

	if ((hasDepthAttachment == true || hasStencilAttachment == true) &&
		_clearAttachments[colorAttachmentCount] == true)
	{
		DepthStencilClear value =
			clearValues[colorAttachmentCount].depthStencil;

		VkClearValue clearValue;
		clearValue.depthStencil.depth = value.depth;
		clearValue.depthStencil.stencil = value.stencil;

		VkImageAspectFlags aspectMask;

		if (hasDepthAttachment == true)
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilAttachment == true)
			aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

		VkClearAttachment clearAttachment = {
			aspectMask,
			(uint32_t)colorAttachmentCount,
			clearValue,
		};

		clearAttachments[clearAttachmentCount++] = clearAttachment;
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
		(uint32_t)clearAttachmentCount,
		clearAttachments,
		1,
		&clearRect);
	free(clearAttachments);
}
#endif

inline static void destroyGlFramebuffer(
	Framebuffer framebuffer)
{
	if (framebuffer == NULL)
		return;

	if (framebuffer->gl.isDefault == false)
	{
		free(framebuffer->gl.colorAttachments);

		makeWindowContextCurrent(
			framebuffer->gl.window);
		glDeleteFramebuffers(
			GL_ONE,
			&framebuffer->gl.handle);
		assertOpenGL();
	}

	assert(framebuffer->gl.pipelineCount == 0);

	free(framebuffer->gl.pipelines);
	free(framebuffer);
}

inline static Framebuffer createDefaultGlFramebuffer(
	Window window,
	Vec2U size)
{
	Framebuffer framebuffer = calloc(1,
		sizeof(Framebuffer_T));

	if (framebuffer == NULL)
		return NULL;

	framebuffer->gl.window = window;
	framebuffer->gl.size = size;
	framebuffer->gl.colorAttachments = NULL;
	framebuffer->gl.colorAttachmentCount = 1;
	framebuffer->gl.depthStencilAttachment = NULL;
	framebuffer->gl.isDefault = true;

	Pipeline* pipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		free(framebuffer);
		return NULL;
	}

	framebuffer->gl.pipelines = pipelines;
	framebuffer->gl.pipelineCapacity = MPGX_DEFAULT_CAPACITY;
	framebuffer->gl.pipelineCount = 0;
	framebuffer->gl.handle = GL_ZERO;
	return framebuffer;
}
inline static Framebuffer createGlFramebuffer(
	Window window,
	Vec2U size,
	bool useBeginClear,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity)
{
	Framebuffer framebuffer = calloc(1,
		sizeof(Framebuffer_T));

	if (framebuffer == NULL)
		return NULL;

	framebuffer->gl.isDefault = false;
	framebuffer->gl.window = window;
	framebuffer->gl.size = size;
	framebuffer->gl.useBeginClear = useBeginClear;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenFramebuffers(
		GL_ONE,
		&handle);

	framebuffer->gl.handle = handle;

	glBindFramebuffer(
		GL_FRAMEBUFFER,
		handle);

	Image* colorAttachments;

	if (colorAttachmentCount != 0)
	{
		colorAttachments = malloc(
			colorAttachmentCount * sizeof(Image));

		if (colorAttachments == NULL)
		{
			destroyGlFramebuffer(framebuffer);
			return NULL;
		}

		GLenum* drawBuffers = malloc(
			sizeof(GLenum) * colorAttachmentCount);

		if (drawBuffers == NULL)
		{
			destroyGlFramebuffer(framebuffer);
			return NULL;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = _colorAttachments[i];

			assert(colorAttachment != NULL);
			assert(colorAttachment->gl.size.x == size.x &&
				colorAttachment->gl.size.y == size.y);
			assert(colorAttachment->gl.window == window);

			ImageFormat format = colorAttachment->gl.format;
			GLenum glAttachment = GL_COLOR_ATTACHMENT0 + (GLenum)i;

			switch (format)
			{
			default:
				free(drawBuffers);
				destroyGlFramebuffer(framebuffer);
				return NULL;
			case R8_UNORM_IMAGE_FORMAT:
			case R8G8B8A8_UNORM_IMAGE_FORMAT:
			case R8G8B8A8_SRGB_IMAGE_FORMAT:
			case R16G16B16A16_SFLOAT_IMAGE_FORMAT:
				glFramebufferTexture2D(
					GL_FRAMEBUFFER,
					glAttachment,
					colorAttachment->gl.glType,
					colorAttachment->gl.handle,
					GL_ZERO);
				break;
			}

			colorAttachments[i] = colorAttachment;
			drawBuffers[i] = glAttachment;
		}

		glDrawBuffers(
			(GLsizei)colorAttachmentCount,
			drawBuffers);
		free(drawBuffers);
	}
	else
	{
		colorAttachments = NULL;

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	framebuffer->gl.colorAttachments = colorAttachments;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;

	if (depthStencilAttachment != NULL)
	{
		ImageFormat format = depthStencilAttachment->gl.format;

		switch (format)
		{
		default:
			destroyGlFramebuffer(framebuffer);
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

	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;

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

		destroyGlFramebuffer(framebuffer);
		return NULL;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlFramebuffer(framebuffer);
		return NULL;
	}

	Pipeline* pipelines = malloc(
		pipelineCapacity * sizeof(Pipeline));

	if (pipelines == NULL)
	{
		destroyGlFramebuffer(framebuffer);
		return NULL;
	}

	framebuffer->gl.pipelines = pipelines;
	framebuffer->gl.pipelineCapacity = pipelineCapacity;
	framebuffer->gl.pipelineCount = 0;
	return framebuffer;
}

inline static bool setGlFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2U size,
	bool useBeginClear,
	Image* _colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	Image* colorAttachments;

	if (colorAttachmentCount != 0)
	{
		colorAttachments = malloc(
			colorAttachmentCount * sizeof(Image));

		if (colorAttachments == NULL)
			return false;

		Window window = framebuffer->gl.window;

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = _colorAttachments[i];

			assert(colorAttachment != NULL);
			assert(
				colorAttachment->gl.size.x == size.x &&
				colorAttachment->gl.size.y == size.y);
			assert(colorAttachment->gl.window == window);

			colorAttachments[i] = colorAttachment;
		}
	}
	else
	{
		colorAttachments = NULL;
	}

	Pipeline* pipelines = framebuffer->gl.pipelines;
	size_t pipelineCount = framebuffer->gl.pipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		Pipeline pipeline = pipelines[i];

		OnPipelineHandleResize onResize =
			getPipelineOnHandleResize(pipeline);

		bool result = onResize(
			pipeline,
			size,
			NULL);

		if (result == false)
		{
			free(colorAttachments);
			return false;
		}
	}

	free(framebuffer->gl.colorAttachments);

	framebuffer->gl.size = size;
	framebuffer->gl.useBeginClear = useBeginClear;
	framebuffer->gl.colorAttachments = colorAttachments;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;
	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;
	return true;
}

inline static void beginGlFramebufferRender(
	GLuint framebuffer,
	Vec2U size,
	size_t colorAttachmentCount,
	bool hasDepthAttachment,
	bool hasStencilAttachment,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	glBindFramebuffer(
		GL_FRAMEBUFFER,
		framebuffer);
	glViewport(
		0, 0,
		(GLsizei)size.x,
		(GLsizei)size.y);
	glDisable(GL_SCISSOR_TEST);

	if (clearValueCount != 0)
	{
		if (colorAttachmentCount != 0)
		{
			glColorMask(
				GL_TRUE, GL_TRUE,
				GL_TRUE, GL_TRUE);

			for (size_t i = 0; i < colorAttachmentCount; i++)
			{
				glClearBufferfv(
					GL_COLOR,
					(GLint)i,
					(const GLfloat*)&clearValues[i].color);
			}
		}

		if (hasDepthAttachment == true || hasStencilAttachment == true)
		{
			DepthStencilClear value =
				clearValues[colorAttachmentCount].depthStencil;

			if (hasDepthAttachment == true && hasStencilAttachment == true)
			{
				glDepthMask(GL_TRUE);
				glStencilMask(UINT32_MAX);

				glClearBufferfi(
					GL_DEPTH_STENCIL,
					0,
					value.depth,
					(GLint)value.stencil);
			}
			else if (hasDepthAttachment == true)
			{
				glDepthMask(GL_TRUE);

				glClearBufferfv(
					GL_DEPTH,
					0,
					&value.depth);
			}
			else
			{
				glStencilMask(UINT32_MAX);

				glClearBufferiv(
					GL_STENCIL,
					0,
					(const GLint*)&value.stencil);
			}
		}
	}

	assertOpenGL();
}
inline static void endGlFramebufferRender()
{
	assertOpenGL();
}
inline static void clearGlFramebuffer(
	Vec2U size,
	size_t colorAttachmentCount,
	bool hasDepthAttachment,
	bool hasStencilAttachment,
	const bool* clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	glViewport(0, 0,
		(GLsizei)size.x,
		(GLsizei)size.y);
	glDisable(GL_SCISSOR_TEST);

	if (clearValueCount != 0)
	{
		if (colorAttachmentCount != 0)
		{
			glColorMask(
				GL_TRUE, GL_TRUE,
				GL_TRUE, GL_TRUE);

			for (size_t i = 0; i < colorAttachmentCount; i++)
			{
				if (clearAttachments[i] == false)
					continue;

				glClearBufferfv(
					GL_COLOR,
					(GLint)i,
					(const GLfloat*)&clearValues[i].color);
			}
		}

		if ((hasDepthAttachment == true || hasStencilAttachment == true) &&
			clearAttachments[colorAttachmentCount] == true)
		{
			DepthStencilClear value =
				clearValues[colorAttachmentCount].depthStencil;

			if (hasDepthAttachment == true && hasStencilAttachment == true)
			{
				glDepthMask(GL_TRUE);
				glStencilMask(UINT32_MAX);

				glClearBufferfi(
					GL_DEPTH_STENCIL,
					0,
					value.depth,
					(GLint)value.stencil);
			}
			else if (hasDepthAttachment == true)
			{
				glDepthMask(GL_TRUE);

				glClearBufferfv(
					GL_DEPTH,
					0,
					&value.depth);
			}
			else
			{
				glStencilMask(UINT32_MAX);

				glClearBufferiv(
					GL_STENCIL,
					0,
					(const GLint*)&value.stencil);
			}
		}
	}

	assertOpenGL();
}
