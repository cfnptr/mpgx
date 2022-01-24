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

#pragma once
#include "mpgx/_source/vulkan.h"
#include "cmmt/common.h"

#include <string.h>

#if MPGX_SUPPORT_VULKAN
typedef struct VkSwapchainBuffer
{
	VkImage image;
	VkImageView imageView;
	VkFramebuffer framebuffer;
	VkCommandBuffer graphicsCommandBuffer;
	VkCommandBuffer presentCommandBuffer;
} VkSwapchainBuffer;
typedef struct VkSwapchain_T
{
	VkSwapchainKHR handle;
	VkImage depthImage;
	VmaAllocation depthAllocation;
	VkImageView depthImageView;
	VkRenderPass renderPass;
	VkSwapchainBuffer* buffers;
	uint32_t bufferCount;
} VkSwapchain_T;

typedef VkSwapchain_T* VkSwapchain;

inline static bool getBestVkSurfaceFormat(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR* surfaceFormat)
{
	assert(physicalDevice);
	assert(surface);
	assert(surfaceFormat);

	uint32_t formatCount;

	VkResult vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice,
		surface,
		&formatCount,
		NULL);

	if (vkResult != VK_SUCCESS)
		return false;

	VkSurfaceFormatKHR* formats = malloc(
		formatCount * sizeof(VkSurfaceFormatKHR));

	if (!formats)
		return false;

	vkResult = vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice,
		surface,
		&formatCount,
		formats);

	if (vkResult != VK_SUCCESS)
	{
		free(formats);
		return false;
	}

	for (uint32_t i = 0; i < formatCount; i++)
	{
		VkSurfaceFormatKHR format = formats[i];

		if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			free(formats);
			*surfaceFormat = format;
			return true;
		}
	}
	for (uint32_t i = 0; i < formatCount; i++)
	{
		VkSurfaceFormatKHR format = formats[i];

		if (format.format == VK_FORMAT_B8G8R8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			free(formats);
			*surfaceFormat = format;
			return true;
		}
	}

	*surfaceFormat = formats[0];
	free(formats);
	return true;
}
inline static bool getBestVkPresentMode(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkPresentModeKHR* presentMode)
{
	assert(physicalDevice);
	assert(surface);
	assert(presentMode);

	uint32_t modeCount;

	VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice,
		surface,
		&modeCount,
		NULL);

	if (result != VK_SUCCESS)
		return false;

	VkPresentModeKHR* modes = malloc(
		modeCount * sizeof(VkPresentModeKHR));

	if (!modes)
		return false;

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice,
		surface,
		&modeCount,
		modes);

	if (result != VK_SUCCESS)
	{
		free(modes);
		return false;
	}

	VkPresentModeKHR bestPresentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (uint32_t i = 0; i < modeCount; i++)
	{
		if (modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			bestPresentMode = modes[i];
			break;
		}
	}
	for (uint32_t i = 0; i < modeCount; i++)
	{
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			bestPresentMode = modes[i];
			break;
		}
	}

	free(modes);

	*presentMode = bestPresentMode;
	return true;
}
inline static VkExtent2D getBestVkSurfaceExtent(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities,
	Vec2I framebufferSize)
{
	assert(surfaceCapabilities);
	assert(framebufferSize.x > 0);
	assert(framebufferSize.y > 0);

	if (surfaceCapabilities->currentExtent.width == UINT32_MAX)
	{
		VkExtent2D surfaceExtent = {
			clamp(framebufferSize.x,
				surfaceCapabilities->minImageExtent.width,
				surfaceCapabilities->maxImageExtent.width),
			clamp(framebufferSize.y,
				surfaceCapabilities->minImageExtent.height,
				surfaceCapabilities->maxImageExtent.height),
		};

		return surfaceExtent;
	}
	else
	{
		return surfaceCapabilities->currentExtent;
	}
}
inline static uint32_t getBestVkImageCount(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
	assert(surfaceCapabilities);

	uint32_t imageCount = surfaceCapabilities->minImageCount + 1;
	uint32_t maxImageCount = surfaceCapabilities->maxImageCount;

	if (maxImageCount > 0 && imageCount > maxImageCount)
		imageCount = maxImageCount;

	return imageCount;
}
inline static VkSurfaceTransformFlagBitsKHR getBestVkSurfaceTransform(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
	assert(surfaceCapabilities);
	// TODO: mobile device rotation

	if(surfaceCapabilities->supportedTransforms &
		VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR)
	{
		return VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR;
	}
	else
	{
		return surfaceCapabilities->currentTransform;
	}
}
inline static bool getBestVkCompositeAlpha(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities,
	VkCompositeAlphaFlagBitsKHR* compositeAlpha)
{
	assert(surfaceCapabilities);
	assert(compositeAlpha);
	// TODO: transparent window creation

	VkCompositeAlphaFlagsKHR supportedCompositeAlpha =
		surfaceCapabilities->supportedCompositeAlpha;

	if (supportedCompositeAlpha &
		VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR)
	{
		*compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
		return true;
	}
	else if (supportedCompositeAlpha &
		VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR)
	{
		*compositeAlpha = VK_COMPOSITE_ALPHA_PRE_MULTIPLIED_BIT_KHR;
		return true;
	}
	else if (supportedCompositeAlpha &
		VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR)
	{
		*compositeAlpha = VK_COMPOSITE_ALPHA_POST_MULTIPLIED_BIT_KHR;
		return true;
	}
	else if (supportedCompositeAlpha &
		VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR)
	{
		*compositeAlpha = VK_COMPOSITE_ALPHA_INHERIT_BIT_KHR;
		return true;
	}
	else
	{
		return false;
	}
}

inline static MpgxResult createVkSwapchainHandle(
	VkSurfaceKHR surface,
	VkDevice device,
	uint32_t imageCount,
	VkSurfaceFormatKHR surfaceFormat,
	VkExtent2D extent,
	VkSurfaceTransformFlagBitsKHR transform,
	VkCompositeAlphaFlagBitsKHR compositeAlpha,
	VkPresentModeKHR presentMode,
	VkSwapchainKHR oldSwapchain,
	VkSwapchainKHR* swapchain)
{
	assert(surface);
	assert(device);
	assert(imageCount > 0);
	assert(extent.width > 0);
	assert(extent.height > 0);
	assert(swapchain);

	VkSwapchainCreateInfoKHR createInfo = {
		VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
		NULL,
		0,
		surface,
		imageCount,
		surfaceFormat.format,
		surfaceFormat.colorSpace,
		extent,
		1,
		VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		transform,
		compositeAlpha,
		presentMode,
		true,
		oldSwapchain
	};

	VkSwapchainKHR swapchainInstance;

	VkResult vkResult = vkCreateSwapchainKHR(
		device,
		&createInfo,
		NULL,
		&swapchainInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*swapchain = swapchainInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static bool getBestVkDepthFormat(
	VkPhysicalDevice physicalDevice,
	bool useStencilBuffer,
	VkFormat* depthFormat)
{
	assert(physicalDevice);
	assert(depthFormat);

	VkFormatProperties properties;

	if (!useStencilBuffer)
	{
		vkGetPhysicalDeviceFormatProperties(
			physicalDevice,
			VK_FORMAT_D32_SFLOAT,
			&properties);

		if(properties.optimalTilingFeatures &
		   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = VK_FORMAT_D32_SFLOAT;
			return true;
		}
	}

	vkGetPhysicalDeviceFormatProperties(
		physicalDevice,
		VK_FORMAT_D24_UNORM_S8_UINT,
		&properties);

	if(properties.optimalTilingFeatures &
	   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		*depthFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		return true;
	}

	vkGetPhysicalDeviceFormatProperties(
		physicalDevice,
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		&properties);

	if(properties.optimalTilingFeatures &
	   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		*depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		return true;
	}

	if (!useStencilBuffer)
	{
		vkGetPhysicalDeviceFormatProperties(
			physicalDevice,
			VK_FORMAT_D16_UNORM,
			&properties);

		if(properties.optimalTilingFeatures &
		   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
		{
			*depthFormat = VK_FORMAT_D16_UNORM;
			return true;
		}
	}

	vkGetPhysicalDeviceFormatProperties(
		physicalDevice,
		VK_FORMAT_D16_UNORM_S8_UINT,
		&properties);

	if(properties.optimalTilingFeatures &
	   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		*depthFormat = VK_FORMAT_D16_UNORM_S8_UINT;
		return true;
	}

	return false;
}

inline static MpgxResult createVkDepthImage(
	VmaAllocator allocator,
	VkFormat format,
	VkExtent2D extent,
	VkImage* depthImage,
	VmaAllocation* depthAllocation)
{
	assert(allocator);
	assert(depthImage);
	assert(depthAllocation);

	VkImageCreateInfo imageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		VK_IMAGE_TYPE_2D,
		format,
		{ extent.width, extent.height, 1, },
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		VK_IMAGE_TILING_OPTIMAL,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocationCreateInfo;
	memset(&allocationCreateInfo, 0, sizeof(VmaAllocationCreateInfo));

	allocationCreateInfo.flags =
		VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT |
		VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

	VkImage depthImageInstance;
	VmaAllocation depthAllocationInstance;

	VkResult vkResult = vmaCreateImage(
		allocator,
		&imageCreateInfo,
		&allocationCreateInfo,
		&depthImageInstance,
		&depthAllocationInstance,
		NULL);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*depthImage = depthImageInstance;
	*depthAllocation = depthAllocationInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDepthImageView(
	VkDevice device,
	VkImage image,
	VkFormat format,
	VkImageView* imageView)
{
	assert(device);
	assert(image);
	assert(imageView);

	VkImageViewCreateInfo createInfo = {
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
			VK_COMPONENT_SWIZZLE_IDENTITY
		},
		{
			VK_IMAGE_ASPECT_DEPTH_BIT,
			0,
			1,
			0,
			1,
		}
	};

	VkImageView imageViewInstance;

	VkResult vkResult = vkCreateImageView(
		device,
		&createInfo,
		NULL,
		&imageViewInstance);

	if(vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*imageView = imageViewInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkRenderPass(
	VkDevice device,
	VkFormat colorFormat,
	VkFormat depthFormat,
	VkRenderPass* renderPass)
{
	assert(device);
	assert(renderPass);

	VkAttachmentDescription attachmentDescriptions[2] = {
		{
			0,
			colorFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_STORE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		},
		{
			0,
			depthFormat,
			VK_SAMPLE_COUNT_1_BIT,
			VK_ATTACHMENT_LOAD_OP_CLEAR,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_ATTACHMENT_LOAD_OP_DONT_CARE,
			VK_ATTACHMENT_STORE_OP_DONT_CARE,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
		},
	};

	VkAttachmentReference colorAttachmentReference = {
		0,
		VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
	};
	VkAttachmentReference depthAttachmentReference = {
		1,
		VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL,
	};

	VkSubpassDescription subpassDescription = {
		0,
		VK_PIPELINE_BIND_POINT_GRAPHICS,
		0,
		NULL,
		1,
		&colorAttachmentReference,
		NULL,
		&depthAttachmentReference,
		0,
		NULL
	};
	VkSubpassDependency subpassDependency = {
		VK_SUBPASS_EXTERNAL,
		0,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT,
		0,
		VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT,
		0,
	};
	VkRenderPassCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
		NULL,
		0,
		2,
		attachmentDescriptions,
		1,
		&subpassDescription,
		1,
		&subpassDependency
	};

	VkRenderPass renderPassInstance;

	VkResult vkResult = vkCreateRenderPass(
		device,
		&createInfo,
		NULL,
		&renderPassInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*renderPass = renderPassInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkSwapchainBuffers(
	VkDevice device,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkSwapchainBuffer* buffers,
	uint32_t bufferCount)
{
	assert(device);
	assert(graphicsCommandPool);
	assert(presentCommandPool);

	assert(bufferCount == 0 ||
		(bufferCount > 0 && buffers));

	for (uint32_t i = 0; i < bufferCount; i++)
	{
		VkSwapchainBuffer* buffer = &buffers[i];

		if (graphicsCommandPool != presentCommandPool)
		{
			vkFreeCommandBuffers(
				device,
				presentCommandPool,
				1,
				&buffer->presentCommandBuffer);
			vkFreeCommandBuffers(
				device,
				graphicsCommandPool,
				1,
				&buffer->graphicsCommandBuffer);
		}
		else
		{
			vkFreeCommandBuffers(
				device,
				graphicsCommandPool,
				1,
				&buffer->graphicsCommandBuffer);
		}

		vkDestroyFramebuffer(
			device,
			buffer->framebuffer,
			NULL);
		vkDestroyImageView(
			device,
			buffer->imageView,
			NULL);
	}

	free(buffers);
}
inline static MpgxResult createVkSwapchainBuffers(
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	VkDevice device,
	VkSwapchainKHR swapchain,
	VkRenderPass renderPass,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkFormat surfaceFormat,
	VkImageView depthImageView,
	VkExtent2D surfaceExtent,
	VkSwapchainBuffer** buffers,
	uint32_t* bufferCount)
{
	assert(device);
	assert(swapchain);
	assert(renderPass);
	assert(graphicsCommandPool);
	assert(presentCommandPool);
	assert(depthImageView);
	assert(surfaceExtent.width > 0);
	assert(surfaceExtent.height > 0);
	assert(buffers);
	assert(bufferCount);

	uint32_t imageCount;

	VkResult vkResult = vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&imageCount,
		NULL);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	VkImage* images = malloc(
		imageCount * sizeof(VkImage));

	if (!images)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	vkResult = vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&imageCount,
		images);

	if (vkResult != VK_SUCCESS)
	{
		free(images);
		return vkToMpgxResult(vkResult);
	}

	VkSwapchainBuffer* bufferArray = malloc(
		imageCount * sizeof(VkSwapchainBuffer));

	if (!bufferArray)
	{
		free(images);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < imageCount; i++)
	{
		// TODO: Move structures outside for scope
		VkImageViewCreateInfo imageViewCreateInfo = {
			VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
			NULL,
			0,
			images[i],
			VK_IMAGE_VIEW_TYPE_2D,
			surfaceFormat,
			{
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY,
				VK_COMPONENT_SWIZZLE_IDENTITY
			},
			{
				VK_IMAGE_ASPECT_COLOR_BIT,
				0,
				1,
				0,
				1,
			},
		};

		VkImageView imageView;

		vkResult = vkCreateImageView(
			device,
			&imageViewCreateInfo,
			NULL,
			&imageView);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkSwapchainBuffers(
				device,
				graphicsCommandPool,
				presentCommandPool,
				bufferArray,
				i);
			free(images);
			return vkToMpgxResult(vkResult);
		}

		VkImageView imageViews[2] = {
			imageView,
			depthImageView,
		};

		VkFramebufferCreateInfo framebufferCreateInfo = {
			VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
			NULL,
			0,
			renderPass,
			2,
			imageViews,
			surfaceExtent.width,
			surfaceExtent.height,
			1,
		};

		VkFramebuffer framebuffer;

		vkResult = vkCreateFramebuffer(
			device,
			&framebufferCreateInfo,
			NULL,
			&framebuffer);

		if (vkResult != VK_SUCCESS)
		{
			vkDestroyImageView(
				device,
				imageView,
				NULL);
			destroyVkSwapchainBuffers(
				device,
				graphicsCommandPool,
				presentCommandPool,
				bufferArray,
				i);
			free(images);
			return vkToMpgxResult(vkResult);
		}

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			graphicsCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		VkCommandBuffer graphicsCommandBuffer;
		VkCommandBuffer presentCommandBuffer;

		vkResult = vkAllocateCommandBuffers(
			device,
			&commandBufferAllocateInfo,
			&graphicsCommandBuffer);

		if (vkResult != VK_SUCCESS)
		{
			vkDestroyFramebuffer(
				device,
				framebuffer,
				NULL);
			vkDestroyImageView(
				device,
				imageView,
				NULL);
			destroyVkSwapchainBuffers(
				device,
				graphicsCommandPool,
				presentCommandPool,
				bufferArray,
				i);
			free(images);
			return vkToMpgxResult(vkResult);
		}

		if (graphicsCommandPool != presentCommandPool)
		{
			commandBufferAllocateInfo.commandPool = presentCommandPool;

			vkResult = vkAllocateCommandBuffers(
				device,
				&commandBufferAllocateInfo,
				&presentCommandBuffer);

			if (vkResult != VK_SUCCESS)
			{
				vkFreeCommandBuffers(
					device,
					graphicsCommandPool,
					1,
					&graphicsCommandBuffer);
				vkDestroyFramebuffer(
					device,
					framebuffer,
					NULL);
				vkDestroyImageView(
					device,
					imageView,
					NULL);
				destroyVkSwapchainBuffers(
					device,
					graphicsCommandPool,
					presentCommandPool,
					bufferArray,
					i);
				free(images);
				return vkToMpgxResult(vkResult);
			}

			VkCommandBufferBeginInfo commandBufferBeginInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				NULL,
				VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
				NULL,
			};

			vkResult = vkBeginCommandBuffer(
				presentCommandBuffer,
				&commandBufferBeginInfo);

			if (vkResult != VK_SUCCESS)
			{
				vkFreeCommandBuffers(
					device,
					presentCommandPool,
					1,
					&presentCommandBuffer);
				vkFreeCommandBuffers(
					device,
					graphicsCommandPool,
					1,
					&graphicsCommandBuffer);
				vkDestroyFramebuffer(
					device,
					framebuffer,
					NULL);
				vkDestroyImageView(
					device,
					imageView,
					NULL);
				destroyVkSwapchainBuffers(
					device,
					graphicsCommandPool,
					presentCommandPool,
					bufferArray,
					i);
				free(images);
				return vkToMpgxResult(vkResult);
			}

			VkImageMemoryBarrier imageMemoryBarrier = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				VK_ACCESS_NONE_KHR,
				VK_ACCESS_NONE_KHR,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
				graphicsQueueFamilyIndex,
				presentQueueFamilyIndex,
				images[i],
				{
					VK_IMAGE_ASPECT_COLOR_BIT,
					0,
					1,
					0,
					1,
				},
			};

			vkCmdPipelineBarrier(
				presentCommandBuffer,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
				0,
				0,
				NULL,
				0,
				NULL,
				1,
				&imageMemoryBarrier);

			vkResult = vkEndCommandBuffer(presentCommandBuffer);

			if (vkResult != VK_SUCCESS)
			{
				vkFreeCommandBuffers(
					device,
					presentCommandPool,
					1,
					&presentCommandBuffer);
				vkFreeCommandBuffers(
					device,
					graphicsCommandPool,
					1,
					&graphicsCommandBuffer);
				vkDestroyFramebuffer(
					device,
					framebuffer,
					NULL);
				vkDestroyImageView(
					device,
					imageView,
					NULL);
				destroyVkSwapchainBuffers(
					device,
					graphicsCommandPool,
					presentCommandPool,
					bufferArray,
					i);
				free(images);
				return vkToMpgxResult(vkResult);
			}
		}
		else
		{
			presentCommandBuffer = graphicsCommandBuffer;
		}

		VkSwapchainBuffer buffer = {
			images[i],
			imageView,
			framebuffer,
			graphicsCommandBuffer,
			presentCommandBuffer
		};

		bufferArray[i] = buffer;
	}

	free(images);

	*buffers = bufferArray;
	*bufferCount = imageCount;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkSwapchain(
	VkDevice device,
	VmaAllocator allocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkSwapchain swapchain)
{
	assert(device);
	assert(allocator);
	assert(graphicsCommandPool);
	assert(presentCommandPool);

	if (!swapchain)
		return;

	destroyVkSwapchainBuffers(
		device,
		graphicsCommandPool,
		presentCommandPool,
		swapchain->buffers,
		swapchain->bufferCount);
	vkDestroyRenderPass(
		device,
		swapchain->renderPass,
		NULL);
	vkDestroyImageView(
		device,
		swapchain->depthImageView,
		NULL);
	vmaDestroyImage(
		allocator,
		swapchain->depthImage,
		swapchain->depthAllocation);
	vkDestroySwapchainKHR(
		device,
		swapchain->handle,
		NULL);
	free(swapchain);
}
inline static MpgxResult createVkSwapchain(
	VkSurfaceKHR surface,
	VkPhysicalDevice physicalDevice,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	VkDevice device,
	VmaAllocator allocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	bool useStencilBuffer,
	Vec2I framebufferSize,
	VkSwapchain* vkSwapchain)
{
	assert(surface);
	assert(physicalDevice);
	assert(device);
	assert(allocator);
	assert(graphicsCommandPool);
	assert(presentCommandPool);
	assert(framebufferSize.x > 0);
	assert(framebufferSize.y > 0);
	assert(vkSwapchain);

	VkSwapchain swapchain = calloc(1,
		sizeof(VkSwapchain_T));

	if (!swapchain)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkSurfaceFormatKHR surfaceFormat;

	if (!getBestVkSurfaceFormat(physicalDevice,
		surface, &surfaceFormat))
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkPresentModeKHR presentMode;

	if (!getBestVkPresentMode(physicalDevice,
		surface, &presentMode))
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	VkResult vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		surface,
		&surfaceCapabilities);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkExtent2D surfaceExtent = getBestVkSurfaceExtent(
		&surfaceCapabilities,
		framebufferSize);

	uint32_t imageCount =
		getBestVkImageCount(&surfaceCapabilities);
	VkSurfaceTransformFlagBitsKHR surfaceTransform =
		getBestVkSurfaceTransform(&surfaceCapabilities);

	VkCompositeAlphaFlagBitsKHR compositeAlpha;

	if (!getBestVkCompositeAlpha(
		&surfaceCapabilities,
		&compositeAlpha))
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkSwapchainKHR handle;

	MpgxResult mpgxResult = createVkSwapchainHandle(
		surface,
		device,
		imageCount,
		surfaceFormat,
		surfaceExtent,
		surfaceTransform,
		compositeAlpha,
		presentMode,
		NULL,
		&handle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return mpgxResult;
	}

	swapchain->handle = handle;

	VkFormat depthFormat;

	if (!getBestVkDepthFormat(physicalDevice,
		useStencilBuffer, &depthFormat))
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkImage depthImage;
	VmaAllocation depthAllocation;

	mpgxResult = createVkDepthImage(
		allocator,
		depthFormat,
		surfaceExtent,
		&depthImage,
		&depthAllocation);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return mpgxResult;
	}

	swapchain->depthImage = depthImage;
	swapchain->depthAllocation = depthAllocation;

	VkImageView depthImageView;

	mpgxResult = createVkDepthImageView(
		device,
		depthImage,
		depthFormat,
		&depthImageView);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return mpgxResult;
	}

	swapchain->depthImageView = depthImageView;

	VkRenderPass renderPass;

	mpgxResult = createVkRenderPass(
		device,
		surfaceFormat.format,
		depthFormat,
		&renderPass);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return mpgxResult;
	}

	swapchain->renderPass = renderPass;

	VkSwapchainBuffer* buffers;
	uint32_t bufferCount;

	mpgxResult = createVkSwapchainBuffers(
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		device,
		handle,
		renderPass,
		graphicsCommandPool,
		presentCommandPool,
		surfaceFormat.format,
		depthImageView,
		surfaceExtent,
		&buffers,
		&bufferCount);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkSwapchain(
			device,
			allocator,
			graphicsCommandPool,
			presentCommandPool,
			swapchain);
		return mpgxResult;
	}

	swapchain->buffers = buffers;
	swapchain->bufferCount = bufferCount;

	*vkSwapchain = swapchain;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult resizeVkSwapchain(
	VkSurfaceKHR surface,
	VkPhysicalDevice physicalDevice,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	VkDevice device,
	VmaAllocator allocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkSwapchain swapchain,
	bool useStencilBuffer,
	Vec2I framebufferSize)
{
	assert(surface);
	assert(physicalDevice);
	assert(device);
	assert(allocator);
	assert(graphicsCommandPool);
	assert(presentCommandPool);
	assert(swapchain);
	assert(framebufferSize.x > 0);
	assert(framebufferSize.y > 0);

	VkResult vkResult = vkDeviceWaitIdle(device);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	VkSurfaceFormatKHR surfaceFormat;

	if (!getBestVkSurfaceFormat(physicalDevice,
		surface, &surfaceFormat))
	{
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkPresentModeKHR presentMode;

	if (!getBestVkPresentMode(physicalDevice,
		surface, &presentMode))
	{
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	vkResult = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		surface,
		&surfaceCapabilities);

	if (vkResult != VK_SUCCESS)
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;

	VkExtent2D surfaceExtent = getBestVkSurfaceExtent(
		&surfaceCapabilities,
		framebufferSize);

	uint32_t imageCount =
		getBestVkImageCount(&surfaceCapabilities);
	VkSurfaceTransformFlagBitsKHR surfaceTransform =
		getBestVkSurfaceTransform(&surfaceCapabilities);

	VkCompositeAlphaFlagBitsKHR compositeAlpha;

	if (!getBestVkCompositeAlpha(
		&surfaceCapabilities,
		&compositeAlpha))
	{
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkSwapchainKHR handle;

	MpgxResult mpgxResult = createVkSwapchainHandle(
		surface,
		device,
		imageCount,
		surfaceFormat,
		surfaceExtent,
		surfaceTransform,
		compositeAlpha,
		presentMode,
		swapchain->handle,
		&handle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	VkFormat depthFormat;

	if (!getBestVkDepthFormat(physicalDevice,
		useStencilBuffer, &depthFormat))
	{
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkImage depthImage;
	VmaAllocation depthAllocation;

	mpgxResult = createVkDepthImage(
		allocator,
		depthFormat,
		surfaceExtent,
		&depthImage,
		&depthAllocation);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		return mpgxResult;
	}

	VkImageView depthImageView;

	mpgxResult = createVkDepthImageView(
		device,
		depthImage,
		depthFormat,
		&depthImageView);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyImage(
			allocator,
			depthImage,
			depthAllocation);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		return mpgxResult;
	}

	VkRenderPass renderPass;

	mpgxResult = createVkRenderPass(
		device,
		surfaceFormat.format,
		depthFormat,
		&renderPass);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		vmaDestroyImage(
			allocator,
			depthImage,
			depthAllocation);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		return mpgxResult;
	}

	VkSwapchainBuffer* buffers;
	uint32_t bufferCount;

	mpgxResult = createVkSwapchainBuffers(
		graphicsQueueFamilyIndex,
		presentQueueFamilyIndex,
		device,
		handle,
		renderPass,
		graphicsCommandPool,
		presentCommandPool,
		surfaceFormat.format,
		depthImageView,
		surfaceExtent,
		&buffers,
		&bufferCount);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vkDestroyRenderPass(
			device,
			renderPass,
			NULL);
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		vmaDestroyImage(
			allocator,
			depthImage,
			depthAllocation);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		return mpgxResult;
	}

	destroyVkSwapchainBuffers(
		device,
		graphicsCommandPool,
		presentCommandPool,
		swapchain->buffers,
		swapchain->bufferCount);
	vkDestroyRenderPass(
		device,
		swapchain->renderPass,
		NULL);
	vkDestroyImageView(
		device,
		swapchain->depthImageView,
		NULL);
	vmaDestroyImage(
		allocator,
		swapchain->depthImage,
		swapchain->depthAllocation);
	vkDestroySwapchainKHR(
		device,
		swapchain->handle,
		NULL);

	swapchain->handle = handle;
	swapchain->depthImage = depthImage;
	swapchain->depthAllocation = depthAllocation;
	swapchain->depthImageView = depthImageView;
	swapchain->renderPass = renderPass;
	swapchain->buffers = buffers;
	swapchain->bufferCount = bufferCount;
	return SUCCESS_MPGX_RESULT;
}
#endif
