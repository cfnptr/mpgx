// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	GraphicsPipeline* graphicsPipelines;
	size_t graphicsPipelineCapacity;
	size_t graphicsPipelineCount;
	Vec2U size;
	bool isDefault;
	bool useBeginClear;
} BaseFramebuffer_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkFramebuffer_T
{
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	GraphicsPipeline* graphicsPipelines;
	size_t graphicsPipelineCapacity;
	size_t graphicsPipelineCount;
	Vec2U size;
	bool isDefault;
	bool useBeginClear;
	VkRenderPass renderPass;
	VkFramebuffer handle;
} VkFramebuffer_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlFramebuffer_T
{
	Window window;
	Image* colorAttachments;
	size_t colorAttachmentCount;
	Image depthStencilAttachment;
	GraphicsPipeline* graphicsPipelines;
	size_t graphicsPipelineCapacity;
	size_t graphicsPipelineCount;
	Vec2U size;
	bool isDefault;
	bool useBeginClear;
	GLuint handle;
} GlFramebuffer_T;
#endif
union Framebuffer_T
{
	BaseFramebuffer_T base;
#if MPGX_SUPPORT_VULKAN
	VkFramebuffer_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlFramebuffer_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkGraphicsPipelineCreateData
{
	uint32_t vertexBindingDescriptionCount;
	const VkVertexInputBindingDescription* vertexBindingDescriptions;
	uint32_t vertexAttributeDescriptionCount;
	const VkVertexInputAttributeDescription* vertexAttributeDescriptions;
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pushConstantRanges;
} VkGraphicsPipelineCreateData;

inline static MpgxResult createVkGeneralRenderPass(
	VkDevice device,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	VkRenderPass* renderPass)
{
	assert(device);
	assert(renderPass);
	// TODO: add attachment assertions

	size_t attachmentCount = depthStencilAttachment ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkAttachmentDescription* attachmentDescriptions = malloc(
		attachmentCount * sizeof(VkAttachmentDescription));

	if (!attachmentDescriptions)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkAttachmentReference* colorReferences;

	if (colorAttachmentCount > 0)
	{
		colorReferences = malloc(
			colorAttachmentCount * sizeof(VkAttachmentDescription));

		if (!colorReferences)
		{
			free(attachmentDescriptions);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		VkAttachmentDescription attachmentDescription = {
			0,
			0,
			VK_SAMPLE_COUNT_1_BIT,
			useBeginClear ?
				VK_ATTACHMENT_LOAD_OP_CLEAR :
				VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
		};
		VkAttachmentReference attachmentReference = {
			0,
			VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
		};

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = colorAttachments[i];
			attachmentDescription.format = colorAttachment->vk.vkFormat;
			attachmentDescriptions[i] = attachmentDescription;
			attachmentReference.attachment = (uint32_t)i;
			colorReferences[i] = attachmentReference;
		}
	}
	else
	{
		colorReferences = NULL;
	}

	if (depthStencilAttachment)
	{
		ImageFormat format = depthStencilAttachment->vk.format;

		VkAttachmentLoadOp stencilLoadOp;
		VkAttachmentStoreOp stencilStoreOp;

		switch (format)
		{
		default:
			free(colorReferences);
			free(attachmentDescriptions);
			return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
		case D16_UNORM_IMAGE_FORMAT:
		case D32_SFLOAT_IMAGE_FORMAT:
			stencilLoadOp = VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			stencilStoreOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
			break;
		case D16_UNORM_S8_UINT_IMAGE_FORMAT:
		case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
			stencilLoadOp = useBeginClear ?
				VK_ATTACHMENT_LOAD_OP_CLEAR :
				VK_ATTACHMENT_LOAD_OP_DONT_CARE;
			stencilStoreOp = VK_ATTACHMENT_STORE_OP_STORE;
			break;
		}

		VkAttachmentDescription attachmentDescription = {
			0,
			depthStencilAttachment->vk.vkFormat,
			VK_SAMPLE_COUNT_1_BIT,
			useBeginClear ?
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
		depthStencilAttachment ?
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

	VkRenderPass renderPassInstance;

	VkResult vkResult = vkCreateRenderPass(
		device,
		&renderPassCreateInfo,
		NULL,
		&renderPassInstance);

	free(colorReferences);
	free(attachmentDescriptions);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*renderPass = renderPassInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkShadowRenderPass(
	VkDevice device,
	VkFormat format,
	VkRenderPass* renderPass)
{
	assert(device);
	assert(renderPass);

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

	VkRenderPass renderPassInstance;

	VkResult vkResult = vkCreateRenderPass(
		device,
		&renderPassCreateInfo,
		NULL,
		&renderPassInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*renderPass = renderPassInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkFramebufferHandle(
	VkDevice device,
	VkRenderPass renderPass,
	size_t attachmentCount,
	VkImageView* imageViews,
	Vec2U size,
	VkFramebuffer* handle)
{
	assert(device);
	assert(renderPass);
	assert(attachmentCount > 0);
	assert(imageViews);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(handle);

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

	VkFramebuffer handleInstance;

	VkResult vkResult = vkCreateFramebuffer(
		device,
		&framebufferCreateInfo,
		NULL,
		&handleInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*handle = handleInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkFramebuffer(
	VkDevice device,
	Framebuffer framebuffer,
	bool destroyAttachments)
{
	assert(device);

	if (!framebuffer)
		return;

	if (!framebuffer->vk.isDefault)
	{
		vkDestroyFramebuffer(
			device,
			framebuffer->vk.handle,
			NULL);
		vkDestroyRenderPass(
			device,
			framebuffer->vk.renderPass,
			NULL);

		if (destroyAttachments)
		{
			Image* colorAttachments = framebuffer->vk.colorAttachments;
			size_t colorAttachmentCount = framebuffer->vk.colorAttachmentCount;

			for (size_t i = 0; i < colorAttachmentCount; i++)
				destroyImage(colorAttachments[i]);

			destroyImage(framebuffer->vk.depthStencilAttachment);
		}

		free(framebuffer->vk.colorAttachments);
	}

	assert(framebuffer->vk.graphicsPipelineCount == 0);

	free(framebuffer->vk.graphicsPipelines);
	free(framebuffer);
}

inline static MpgxResult createVkDefaultFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	VkFramebuffer handle,
	Window window,
	Vec2U size,
	Framebuffer* framebuffer)
{
	assert(device);
	assert(renderPass);
	assert(handle);
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(framebuffer);

	Framebuffer framebufferInstance = calloc(1,
		sizeof(Framebuffer_T));

	if (!framebufferInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	framebufferInstance->vk.window = window;
	framebufferInstance->vk.colorAttachments = NULL;
	framebufferInstance->vk.colorAttachmentCount = 1;
	framebufferInstance->vk.depthStencilAttachment = NULL;
	framebufferInstance->vk.size = size;
	framebufferInstance->vk.isDefault = true;
	framebufferInstance->vk.useBeginClear = true;

	GraphicsPipeline* graphicsPipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(GraphicsPipeline));

	if (!graphicsPipelines)
	{
		destroyVkFramebuffer(
			device,
			framebufferInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	framebufferInstance->vk.graphicsPipelines = graphicsPipelines;
	framebufferInstance->vk.graphicsPipelineCapacity = MPGX_DEFAULT_CAPACITY;
	framebufferInstance->vk.graphicsPipelineCount = 0;
	framebufferInstance->vk.renderPass = renderPass;
	framebufferInstance->vk.handle = handle;

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkFramebuffer(
	VkDevice device,
	VkRenderPass renderPass,
	Window window,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity,
	Framebuffer* framebuffer)
{
	assert(device);
	assert(renderPass);
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(pipelineCapacity > 0);
	assert(framebuffer);
	// TODO: assert attachments

	Framebuffer framebufferInstance = calloc(1,
		sizeof(Framebuffer_T));

	if (!framebufferInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	framebufferInstance->vk.isDefault = false;
	framebufferInstance->vk.window = window;
	framebufferInstance->vk.size = size;
	framebufferInstance->vk.useBeginClear = useBeginClear;

	size_t attachmentCount = depthStencilAttachment ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (!imageViews)
	{
		destroyVkFramebuffer(
			device,
			framebufferInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	Image* colorAttachmentArray;

	if (colorAttachmentCount > 0)
	{
		colorAttachmentArray = malloc(
			colorAttachmentCount * sizeof(Image));

		if (!colorAttachmentArray)
		{
			free(imageViews);
			destroyVkFramebuffer(
				device,
				framebufferInstance,
				false);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = colorAttachments[i];
			colorAttachmentArray[i] = colorAttachment;
			imageViews[i] = colorAttachment->vk.imageView;
		}
	}
	else
	{
		colorAttachmentArray = NULL;
	}

	framebufferInstance->vk.colorAttachments = colorAttachmentArray;
	framebufferInstance->vk.colorAttachmentCount = colorAttachmentCount;

	if (depthStencilAttachment)
	{
		imageViews[colorAttachmentCount] =
			depthStencilAttachment->vk.imageView;
	}

	framebufferInstance->vk.depthStencilAttachment = depthStencilAttachment;

	GraphicsPipeline* graphicsPipelines = malloc(
		pipelineCapacity * sizeof(GraphicsPipeline));

	if (!graphicsPipelines)
	{
		destroyVkFramebuffer(
			device,
			framebufferInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	framebufferInstance->vk.graphicsPipelines = graphicsPipelines;
	framebufferInstance->vk.graphicsPipelineCapacity = pipelineCapacity;
	framebufferInstance->vk.graphicsPipelineCount = 0;

	VkFramebuffer handle;

	MpgxResult mpgxResult = createVkFramebufferHandle(
		device,
		renderPass,
		attachmentCount,
		imageViews,
		size,
		&handle);

	free(imageViews);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkFramebuffer(
			device,
			framebufferInstance,
			false);
		return mpgxResult;
	}

	framebufferInstance->vk.renderPass = renderPass;
	framebufferInstance->vk.handle = handle;

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult recreateVkGraphicsPipelineHandle(
	VkDevice device,
	VkRenderPass renderPass,
	GraphicsPipeline graphicsPipeline,
	size_t colorAttachmentCount,
	const VkGraphicsPipelineCreateData* createData);

inline static MpgxResult setVkFramebufferAttachments(
	VkDevice device,
	VkRenderPass renderPass,
	Framebuffer framebuffer,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	assert(device);
	assert(renderPass);
	assert(framebuffer);
	assert(size.x > 0);
	assert(size.y > 0);
	// TODO: assert attachments

	size_t attachmentCount = depthStencilAttachment ?
		colorAttachmentCount + 1 : colorAttachmentCount;

	VkImageView* imageViews = malloc(
		attachmentCount * sizeof(VkImageView));

	if (!imageViews)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	Image* colorAttachmentArray;

	if (colorAttachmentCount > 0)
	{
		colorAttachmentArray = malloc(
			colorAttachmentCount * sizeof(Image));

		if (!colorAttachmentArray)
		{
			free(imageViews);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = colorAttachments[i];
			colorAttachmentArray[i] = colorAttachment;
			imageViews[i] = colorAttachment->vk.imageView;
		}
	}
	else
	{
		colorAttachmentArray = NULL;
	}

	if (depthStencilAttachment)
	{
		imageViews[colorAttachmentCount] =
			depthStencilAttachment->vk.imageView;
	}

	VkFramebuffer handle;

	MpgxResult mpgxResult = createVkFramebufferHandle(
		device,
		renderPass,
		attachmentCount,
		imageViews,
		size,
		&handle);

	free(imageViews);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		free(colorAttachmentArray);
		return mpgxResult;
	}

	GraphicsPipeline* graphicsPipelines = framebuffer->vk.graphicsPipelines;
	size_t pipelineCount = framebuffer->vk.graphicsPipelineCount;

	for (size_t i = 0; i < pipelineCount; i++)
	{
		GraphicsPipeline graphicsPipeline = graphicsPipelines[i];

		OnGraphicsPipelineResize onResize =
			getGraphicsPipelineOnResize(graphicsPipeline);

		VkGraphicsPipelineCreateData createData;

		mpgxResult = onResize(
			graphicsPipeline,
			size,
			&createData);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyFramebuffer(
				device,
				handle,
				NULL);
			free(colorAttachmentArray);
			return mpgxResult;
		}

		mpgxResult = recreateVkGraphicsPipelineHandle(
			device,
			renderPass,
			graphicsPipeline,
			colorAttachmentCount,
			&createData);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			vkDestroyFramebuffer(
				device,
				handle,
				NULL);
			free(colorAttachmentArray);
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
	framebuffer->vk.colorAttachments = colorAttachmentArray;
	framebuffer->vk.colorAttachmentCount = colorAttachmentCount;
	framebuffer->vk.depthStencilAttachment = depthStencilAttachment;
	framebuffer->vk.handle = handle;
	framebuffer->vk.renderPass = renderPass;
	return SUCCESS_MPGX_RESULT;
}

inline static void beginVkFramebufferRender(
	VkCommandBuffer commandBuffer,
	VkRenderPass renderPass,
	VkFramebuffer framebuffer,
	Vec2U size,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	assert(commandBuffer);
	assert(renderPass);
	assert(framebuffer);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(clearValues);
	assert(clearValueCount > 0);

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
	assert(commandBuffer);
	vkCmdEndRenderPass(commandBuffer);
}
inline static void clearVkFramebuffer(
	VkCommandBuffer commandBuffer,
	Vec2U framebufferSize,
	bool hasDepthAttachment,
	bool hasStencilAttachment,
	const bool* clearAttachments,
	const FramebufferClear* clearValues,
	size_t clearValueCount)
{
	assert(commandBuffer);
	assert(framebufferSize.x > 0);
	assert(framebufferSize.y > 0);
	assert(clearAttachments);
	assert(clearValues);
	assert(clearValueCount > 0);

	// TODO: move allocation to framebuffer object (cache it)
	VkClearAttachment* clearAttachmentArray = malloc(
		clearValueCount * sizeof(VkClearAttachment));

	if (!clearAttachmentArray)
		abort();

	size_t clearAttachmentCount = 0;

	size_t colorAttachmentCount =
		hasDepthAttachment || hasStencilAttachment ?
		clearValueCount - 1 : clearValueCount;

	for (size_t i = 0; i < colorAttachmentCount; i++)
	{
		if (!clearAttachments[i])
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

		clearAttachmentArray[clearAttachmentCount++] = clearAttachment;
	}

	if ((hasDepthAttachment | hasStencilAttachment) &
		clearAttachments[colorAttachmentCount])
	{
		DepthStencilClear value =
			clearValues[colorAttachmentCount].depthStencil;

		VkClearValue clearValue;
		clearValue.depthStencil.depth = value.depth;
		clearValue.depthStencil.stencil = value.stencil;

		VkImageAspectFlags aspectMask;

		if (hasDepthAttachment)
			aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		if (hasStencilAttachment)
			aspectMask = VK_IMAGE_ASPECT_STENCIL_BIT;

		VkClearAttachment clearAttachment = {
			aspectMask,
			(uint32_t)colorAttachmentCount,
			clearValue,
		};

		clearAttachmentArray[clearAttachmentCount++] = clearAttachment;
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
		clearAttachmentArray,
		1,
		&clearRect);
	free(clearAttachmentArray);
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static void destroyGlFramebuffer(
	Framebuffer framebuffer,
	bool destroyAttachments)
{
	if (!framebuffer)
		return;

	if (framebuffer->gl.isDefault == false)
	{
		makeWindowContextCurrent(
			framebuffer->gl.window);
		glDeleteFramebuffers(
			GL_ONE,
			&framebuffer->gl.handle);
		assertOpenGL();

		if (destroyAttachments)
		{
			Image* colorAttachments = framebuffer->gl.colorAttachments;
			size_t colorAttachmentCount = framebuffer->gl.colorAttachmentCount;

			for (size_t i = 0; i < colorAttachmentCount; i++)
				destroyImage(colorAttachments[i]);

			destroyImage(framebuffer->gl.depthStencilAttachment);
		}

		free(framebuffer->gl.colorAttachments);
	}

	assert(framebuffer->gl.graphicsPipelineCount == 0);

	free(framebuffer->gl.graphicsPipelines);
	free(framebuffer);
}

inline static MpgxResult createGlDefaultFramebuffer(
	Window window,
	Vec2U size,
	Framebuffer* framebuffer)
{
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(framebuffer);

	Framebuffer framebufferInstance = calloc(1,
		sizeof(Framebuffer_T));

	if (!framebufferInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	framebufferInstance->gl.window = window;
	framebufferInstance->gl.colorAttachments = NULL;
	framebufferInstance->gl.colorAttachmentCount = 1;
	framebufferInstance->gl.depthStencilAttachment = NULL;
	framebufferInstance->gl.size = size;
	framebufferInstance->gl.isDefault = true;
	framebufferInstance->gl.useBeginClear = true;

	GraphicsPipeline* graphicsPipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(GraphicsPipeline));

	if (!graphicsPipelines)
	{
		free(framebuffer);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	framebufferInstance->gl.graphicsPipelines = graphicsPipelines;
	framebufferInstance->gl.graphicsPipelineCapacity = MPGX_DEFAULT_CAPACITY;
	framebufferInstance->gl.graphicsPipelineCount = 0;
	framebufferInstance->gl.handle = GL_ZERO;

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createGlFramebuffer(
	Window window,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment,
	size_t pipelineCapacity,
	Framebuffer* framebuffer)
{
	assert(window);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(pipelineCapacity > 0);
	assert(framebuffer);
	// TODO: assert attachments

	Framebuffer framebufferInstance = calloc(1,
		sizeof(Framebuffer_T));

	if (!framebufferInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	framebufferInstance->gl.window = window;
	framebufferInstance->gl.size = size;
	framebufferInstance->gl.isDefault = false;
	framebufferInstance->gl.useBeginClear = useBeginClear;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenFramebuffers(
		GL_ONE,
		&handle);

	framebufferInstance->gl.handle = handle;

	glBindFramebuffer(
		GL_FRAMEBUFFER,
		handle);

	Image* colorAttachmentArray;

	if (colorAttachmentCount > 0)
	{
		colorAttachmentArray = malloc(
			colorAttachmentCount * sizeof(Image));

		if (!colorAttachmentArray)
		{
			destroyGlFramebuffer(
				framebufferInstance,
				false);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		GLenum* drawBuffers = malloc(
			sizeof(GLenum) * colorAttachmentCount);

		if (!drawBuffers)
		{
			destroyGlFramebuffer(
				framebufferInstance,
				false);
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		}

		for (size_t i = 0; i < colorAttachmentCount; i++)
		{
			Image colorAttachment = colorAttachments[i];

			ImageFormat format = colorAttachment->gl.format;
			GLenum glAttachment = GL_COLOR_ATTACHMENT0 + (GLenum)i;

			switch (format)
			{
			default:
				free(drawBuffers);
				destroyGlFramebuffer(
					framebufferInstance,
					false);
				return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
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

			colorAttachmentArray[i] = colorAttachment;
			drawBuffers[i] = glAttachment;
		}

		glDrawBuffers(
			(GLsizei)colorAttachmentCount,
			drawBuffers);
		free(drawBuffers);
	}
	else
	{
		colorAttachmentArray = NULL;

		glDrawBuffer(GL_NONE);
		glReadBuffer(GL_NONE);
	}

	framebufferInstance->gl.colorAttachments = colorAttachmentArray;
	framebufferInstance->gl.colorAttachmentCount = colorAttachmentCount;

	if (depthStencilAttachment)
	{
		ImageFormat format = depthStencilAttachment->gl.format;

		switch (format)
		{
		default:
			destroyGlFramebuffer(
				framebufferInstance,
				false);
			return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
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

	framebufferInstance->gl.depthStencilAttachment = depthStencilAttachment;

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

		destroyGlFramebuffer(
			framebufferInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlFramebuffer(
			framebufferInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	GraphicsPipeline* graphicsPipelines = malloc(
		pipelineCapacity * sizeof(GraphicsPipeline));

	if (!graphicsPipelines)
	{
		destroyGlFramebuffer(
			framebufferInstance,
			false);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	framebufferInstance->gl.graphicsPipelines = graphicsPipelines;
	framebufferInstance->gl.graphicsPipelineCapacity = pipelineCapacity;
	framebufferInstance->gl.graphicsPipelineCount = 0;

	*framebuffer = framebufferInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult setGlFramebufferAttachments(
	Framebuffer framebuffer,
	Vec2U size,
	bool useBeginClear,
	Image* colorAttachments,
	size_t colorAttachmentCount,
	Image depthStencilAttachment)
{
	assert(framebuffer);
	assert(size.x > 0);
	assert(size.y > 0);
	// TODO: assert attachments

	Image* colorAttachmentArray;

	if (colorAttachmentCount > 0)
	{
		colorAttachmentArray = malloc(
			colorAttachmentCount * sizeof(Image));

		if (!colorAttachmentArray)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;

		for (size_t i = 0; i < colorAttachmentCount; i++)
			colorAttachmentArray[i] = colorAttachments[i];
	}
	else
	{
		colorAttachmentArray = NULL;
	}

	GraphicsPipeline* graphicsPipelines = framebuffer->gl.graphicsPipelines;
	size_t graphicsPipelineCount = framebuffer->gl.graphicsPipelineCount;

	for (size_t i = 0; i < graphicsPipelineCount; i++)
	{
		GraphicsPipeline graphicsPipeline = graphicsPipelines[i];

		OnGraphicsPipelineResize onResize =
			getGraphicsPipelineOnResize(graphicsPipeline);

		MpgxResult mpgxResult = onResize(
			graphicsPipeline,
			size,
			NULL);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			free(colorAttachmentArray);
			return mpgxResult;
		}
	}

	free(framebuffer->gl.colorAttachments);

	framebuffer->gl.size = size;
	framebuffer->gl.useBeginClear = useBeginClear;
	framebuffer->gl.colorAttachments = colorAttachmentArray;
	framebuffer->gl.colorAttachmentCount = colorAttachmentCount;
	framebuffer->gl.depthStencilAttachment = depthStencilAttachment;
	return SUCCESS_MPGX_RESULT;
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
	assert(framebuffer != GL_ZERO);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(clearValues);
	assert(clearValueCount > 0);

	glBindFramebuffer(
		GL_FRAMEBUFFER,
		framebuffer);
	glViewport(
		0, 0,
		(GLsizei)size.x,
		(GLsizei)size.y);
	glDisable(GL_SCISSOR_TEST);

	if (clearValueCount > 0)
	{
		if (colorAttachmentCount > 0)
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

		if (hasDepthAttachment | hasStencilAttachment)
		{
			DepthStencilClear value =
				clearValues[colorAttachmentCount].depthStencil;

			if (hasDepthAttachment & hasStencilAttachment)
			{
				glDepthMask(GL_TRUE);
				glStencilMask(UINT32_MAX);

				glClearBufferfi(
					GL_DEPTH_STENCIL,
					0,
					value.depth,
					(GLint)value.stencil);
			}
			else if (hasDepthAttachment)
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
	assert(size.x > 0);
	assert(size.y > 0);
	assert(clearAttachments);
	assert(clearValues);
	assert(clearValueCount > 0);

	glViewport(0, 0,
		(GLsizei)size.x,
		(GLsizei)size.y);
	glDisable(GL_SCISSOR_TEST);

	if (clearValueCount > 0)
	{
		if (colorAttachmentCount > 0)
		{
			glColorMask(
				GL_TRUE, GL_TRUE,
				GL_TRUE, GL_TRUE);

			for (size_t i = 0; i < colorAttachmentCount; i++)
			{
				if (!clearAttachments[i])
					continue;

				glClearBufferfv(
					GL_COLOR,
					(GLint)i,
					(const GLfloat*)&clearValues[i].color);
			}
		}

		if ((hasDepthAttachment | hasStencilAttachment) &
			clearAttachments[colorAttachmentCount])
		{
			DepthStencilClear value =
				clearValues[colorAttachmentCount].depthStencil;

			if (hasDepthAttachment & hasStencilAttachment)
			{
				glDepthMask(GL_TRUE);
				glStencilMask(UINT32_MAX);

				glClearBufferfi(
					GL_DEPTH_STENCIL,
					0,
					value.depth,
					(GLint)value.stencil);
			}
			else if (hasDepthAttachment)
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
#endif
