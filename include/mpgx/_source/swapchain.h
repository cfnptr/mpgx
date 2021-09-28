#pragma once
#include "cmmt/common.h"
#include "mpgx/_source/image.h"

typedef struct VkSwapchainBuffer
{
	VkImage image;
	VkImageView imageView;
	VkFramebuffer framebuffer;
	VkCommandBuffer graphicsCommandBuffer;
	VkCommandBuffer presentCommandBuffer;
} VkSwapchainBuffer;

struct VkSwapchain
{
	VkSwapchainKHR handle;
	Image depthImage;
	VkImageView depthImageView;
	VkRenderPass renderPass;
	VkSwapchainBuffer*  buffers;
	uint32_t bufferCount;
};

typedef struct VkSwapchain* VkSwapchain;

inline static bool getBestVkSurfaceFormat(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkSurfaceFormatKHR* _surfaceFormat)
{
	uint32_t formatCount;

	VkResult result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice,
		surface,
		&formatCount,
		NULL);

	if (result != VK_SUCCESS ||
		formatCount == 0)
	{
		return false;
	}

	VkSurfaceFormatKHR* formats = malloc(
		formatCount * sizeof(VkSurfaceFormatKHR));

	if (formats == NULL)
		return false;

	result = vkGetPhysicalDeviceSurfaceFormatsKHR(
		physicalDevice,
		surface,
		&formatCount,
		formats);

	if (result != VK_SUCCESS ||
		formatCount == 0)
	{
		free(formats);
		return false;
	}

	VkSurfaceFormatKHR surfaceFormat = formats[0];

	for (uint32_t i = 0; i < formatCount; i++)
	{
		VkSurfaceFormatKHR format = formats[i];

		if (format.format == VK_FORMAT_R8G8B8A8_SRGB &&
			format.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			surfaceFormat = format;
			break;
		}
	}

	free(formats);

	*_surfaceFormat = surfaceFormat;
	return true;
}
inline static bool getBestVkPresentMode(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkPresentModeKHR* _presentMode)
{
	uint32_t modeCount;

	VkResult result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice,
		surface,
		&modeCount,
		NULL);

	if (result != VK_SUCCESS ||
		modeCount == 0)
	{
		return false;
	}

	VkPresentModeKHR* modes = malloc(
		modeCount * sizeof(VkPresentModeKHR));

	if (modes == NULL)
		return false;

	result = vkGetPhysicalDeviceSurfacePresentModesKHR(
		physicalDevice,
		surface,
		&modeCount,
		modes);

	if (result != VK_SUCCESS ||
		modeCount == 0)
	{
		free(modes);
		return false;
	}

	VkPresentModeKHR presentMode = VK_PRESENT_MODE_FIFO_KHR;

	for (uint32_t i = 0; i < modeCount; i++)
	{
		if (modes[i] == VK_PRESENT_MODE_FIFO_RELAXED_KHR)
		{
			presentMode = modes[i];
			break;
		}
	}
	for (uint32_t i = 0; i < modeCount; i++)
	{
		if (modes[i] == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			presentMode = modes[i];
			break;
		}
	}

	free(modes);

	*_presentMode = presentMode;
	return true;
}
inline static bool getVkSurfaceCapabilities(
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
	return vkGetPhysicalDeviceSurfaceCapabilitiesKHR(
		physicalDevice,
		surface,
		surfaceCapabilities) == VK_SUCCESS;
}
inline static VkExtent2D getBestVkSurfaceExtent(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities,
	Vec2U framebufferSize)
{
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
	uint32_t imageCount = surfaceCapabilities->minImageCount + 1;
	uint32_t maxImageCount = surfaceCapabilities->maxImageCount;

	if (maxImageCount > 0 && imageCount > maxImageCount)
		imageCount = maxImageCount;

	return imageCount;
}
inline static VkSurfaceTransformFlagBitsKHR getBestVkSurfaceTransform(
	const VkSurfaceCapabilitiesKHR* surfaceCapabilities)
{
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

inline static VkSwapchainKHR createVkSwapchainHandle(
	VkSurfaceKHR surface,
	VkDevice device,
	uint32_t imageCount,
	VkSurfaceFormatKHR surfaceFormat,
	VkExtent2D extent,
	VkSurfaceTransformFlagBitsKHR transform,
	VkCompositeAlphaFlagBitsKHR compositeAlpha,
	VkPresentModeKHR presentMode,
	VkSwapchainKHR oldSwapchain)
{
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

	VkSwapchainKHR swapchain;

	VkResult result = vkCreateSwapchainKHR(
		device,
		&createInfo,
		NULL,
		&swapchain);

	if (result != VK_SUCCESS)
		return NULL;

	return swapchain;
}

inline static bool getBestVkDepthFormat(
	VkPhysicalDevice physicalDevice,
	bool useStencilBuffer,
	VkFormat* depthFormat)
{
	VkFormatProperties properties;

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

	if (useStencilBuffer == false)
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

inline static VkImageView createVkDepthImageView(
	VkDevice device,
	VkImage image,
	VkFormat format)
{
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

	VkImageView imageView;

	VkResult result = vkCreateImageView(
		device,
		&createInfo,
		NULL,
		&imageView);

	if(result != VK_SUCCESS)
		return NULL;

	return imageView;
}

inline static VkRenderPass createVkRenderPass(
	VkDevice device,
	VkFormat colorFormat,
	VkFormat depthFormat)
{
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

	VkRenderPass renderPass;

	VkResult result = vkCreateRenderPass(
		device,
		&createInfo,
		NULL,
		&renderPass);

	if (result != VK_SUCCESS)
		return NULL;

	return renderPass;
}

inline static void destroyVkSwapchainBuffers(
	VkDevice device,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkSwapchainBuffer* buffers,
	uint32_t bufferCount)
{
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
inline static bool createVkSwapchainBuffers(
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
	VkSwapchainBuffer** _buffers,
	uint32_t* bufferCount)
{
	uint32_t imageCount;

	VkResult result = vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&imageCount,
		NULL);

	if (result != VK_SUCCESS ||
		imageCount == 0)
	{
		return false;
	}

	VkImage* images = malloc(
		imageCount * sizeof(VkImage));

	if (images == NULL)
		return false;

	result = vkGetSwapchainImagesKHR(
		device,
		swapchain,
		&imageCount,
		images);

	if (result != VK_SUCCESS ||
		imageCount == 0)
	{
		free(images);
		return false;
	}

	VkSwapchainBuffer* buffers = malloc(
		imageCount * sizeof(VkSwapchainBuffer));

	if (buffers == NULL)
	{
		free(images);
		return false;
	}

	for (uint32_t i = 0; i < imageCount; i++)
	{
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

		result = vkCreateImageView(
			device,
			&imageViewCreateInfo,
			NULL,
			&imageView);

		if (result != VK_SUCCESS)
		{
			destroyVkSwapchainBuffers(
				device,
				graphicsCommandPool,
				presentCommandPool,
				buffers,
				i);
			free(buffers);
			free(images);
			return false;
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

		result = vkCreateFramebuffer(
			device,
			&framebufferCreateInfo,
			NULL,
			&framebuffer);

		if (result != VK_SUCCESS)
		{
			vkDestroyImageView(
				device,
				imageView,
				NULL);
			destroyVkSwapchainBuffers(
				device,
				graphicsCommandPool,
				presentCommandPool,
				buffers,
				i);
			free(buffers);
			free(images);
			return false;
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

		result = vkAllocateCommandBuffers(
			device,
			&commandBufferAllocateInfo,
			&graphicsCommandBuffer);

		if (result != VK_SUCCESS)
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
				buffers,
				i);
			free(buffers);
			free(images);
			return false;
		}

		if (graphicsCommandPool != presentCommandPool)
		{
			commandBufferAllocateInfo.commandPool = presentCommandPool;

			result = vkAllocateCommandBuffers(
				device,
				&commandBufferAllocateInfo,
				&presentCommandBuffer);

			if (result != VK_SUCCESS)
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
					buffers,
					i);
				free(buffers);
				free(images);
				return false;
			}

			VkCommandBufferBeginInfo commandBufferBeginInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
				NULL,
				VK_COMMAND_BUFFER_USAGE_SIMULTANEOUS_USE_BIT,
				NULL,
			};

			result = vkBeginCommandBuffer(
				presentCommandBuffer,
				&commandBufferBeginInfo);

			if (result != VK_SUCCESS)
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
					buffers,
					i);
				free(buffers);
				free(images);
				return false;
			}

			VkImageMemoryBarrier imageMemoryBarrier = {
				VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
				NULL,
				0,
				0,
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

			result = vkEndCommandBuffer(presentCommandBuffer);

			if (result != VK_SUCCESS)
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
					buffers,
					i);
				free(buffers);
				free(images);
				return false;
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

		buffers[i] = buffer;
	}

	free(images);

	*_buffers = buffers;
	*bufferCount = imageCount;
	return true;
}

inline static VkSwapchain createVkSwapchain(
	Window window,
	VkSurfaceKHR surface,
	VkPhysicalDevice physicalDevice,
	uint32_t graphicsQueueFamilyIndex,
	uint32_t presentQueueFamilyIndex,
	VkDevice device,
	VmaAllocator allocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	bool useStencilBuffer,
	Vec2U framebufferSize)
{
	VkSwapchain swapchain = malloc(
		sizeof(struct VkSwapchain));

	if (swapchain == NULL)
		return NULL;

	VkSurfaceFormatKHR surfaceFormat;

	bool result = getBestVkSurfaceFormat(
		physicalDevice,
		surface,
		&surfaceFormat);

	if (result == false)
	{
		free(swapchain);
		return NULL;
	}

	VkPresentModeKHR presentMode;

	result = getBestVkPresentMode(
		physicalDevice,
		surface,
		&presentMode);

	if (result == false)
	{
		free(swapchain);
		return NULL;
	}

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	result = getVkSurfaceCapabilities(
		physicalDevice,
		surface,
		&surfaceCapabilities);

	if (result == false)
	{
		free(swapchain);
		return NULL;
	}

	VkExtent2D surfaceExtent = getBestVkSurfaceExtent(
		&surfaceCapabilities,
		framebufferSize);

	uint32_t imageCount =
		getBestVkImageCount(&surfaceCapabilities);
	VkSurfaceTransformFlagBitsKHR surfaceTransform =
		getBestVkSurfaceTransform(&surfaceCapabilities);

	VkCompositeAlphaFlagBitsKHR compositeAlpha;

	result = getBestVkCompositeAlpha(
		&surfaceCapabilities,
		&compositeAlpha);

	if (result == false)
	{
		free(swapchain);
		return NULL;
	}

	VkSwapchainKHR handle = createVkSwapchainHandle(
		surface,
		device,
		imageCount,
		surfaceFormat,
		surfaceExtent,
		surfaceTransform,
		compositeAlpha,
		presentMode,
		NULL);

	if (handle == NULL)
	{
		free(swapchain);
		return NULL;
	}

	VkFormat depthFormat;

	result = getBestVkDepthFormat(
		physicalDevice,
		useStencilBuffer,
		&depthFormat);

	if (result == false)
	{
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		free(swapchain);
		return NULL;
	}

	Vec3U imageSize = vec3U(
		surfaceExtent.width,
		surfaceExtent.height,
		1);

	// TODO: possibly optimize with fully dedicated memory block
	Image depthImage = createVkImage(
		allocator,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		depthFormat,
		window,
		IMAGE_2D_TYPE,
		IMAGE_FORMAT_COUNT,
		imageSize);

	if (depthImage == NULL)
	{
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		free(swapchain);
		return NULL;
	}

	VkImageView depthImageView = createVkDepthImageView(
		device,
		depthImage->vk.handle,
		depthFormat);

	if (depthImageView == NULL)
	{
		destroyVkImage(
			allocator,
			depthImage);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		free(swapchain);
		return NULL;
	}

	VkRenderPass renderPass = createVkRenderPass(
		device,
		surfaceFormat.format,
		depthFormat);

	if (renderPass == NULL)
	{
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		destroyVkImage(
			allocator,
			depthImage);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		free(swapchain);
		return NULL;
	}

	VkSwapchainBuffer* buffers;
	uint32_t bufferCount;

	result = createVkSwapchainBuffers(
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

	if (result == false)
	{
		vkDestroyRenderPass(
			device,
			renderPass,
			NULL);
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		destroyVkImage(
			allocator,
			depthImage);
		vkDestroySwapchainKHR(
			device,
			handle,
			NULL);
		free(swapchain);
		return NULL;
	}

	swapchain->handle = handle;
	swapchain->depthImage = depthImage;
	swapchain->depthImageView = depthImageView;
	swapchain->renderPass = renderPass;
	swapchain->buffers = buffers;
	swapchain->bufferCount = bufferCount;
	return swapchain;
}
inline static void destroyVkSwapchain(
	VkDevice device,
	VmaAllocator allocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	VkSwapchain swapchain)
{
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
	destroyVkImage(
		allocator,
		swapchain->depthImage);
	vkDestroySwapchainKHR(
		device,
		swapchain->handle,
		NULL);
	free(swapchain);
}

inline static bool resizeVkSwapchain(
	Window window,
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
	Vec2U framebufferSize)
{
	vkDeviceWaitIdle(device);

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
	destroyVkImage(
		allocator,
		swapchain->depthImage);

	swapchain->bufferCount = 0;
	swapchain->buffers = NULL;
	swapchain->renderPass = NULL;
	swapchain->depthImageView = NULL;
	swapchain->depthImage = NULL;

	VkSurfaceFormatKHR surfaceFormat;

	bool result = getBestVkSurfaceFormat(
		physicalDevice,
		surface,
		&surfaceFormat);

	if (result == false)
		return false;

	VkPresentModeKHR presentMode;

	result = getBestVkPresentMode(
		physicalDevice,
		surface,
		&presentMode);

	if (result == false)
		return false;

	VkSurfaceCapabilitiesKHR surfaceCapabilities;

	result = getVkSurfaceCapabilities(
		physicalDevice,
		surface,
		&surfaceCapabilities);

	if (result == false)
		return false;

	VkExtent2D surfaceExtent = getBestVkSurfaceExtent(
		&surfaceCapabilities,
		framebufferSize);

	uint32_t imageCount =
		getBestVkImageCount(&surfaceCapabilities);
	VkSurfaceTransformFlagBitsKHR surfaceTransform =
		getBestVkSurfaceTransform(&surfaceCapabilities);

	VkCompositeAlphaFlagBitsKHR compositeAlpha;

	result = getBestVkCompositeAlpha(
		&surfaceCapabilities,
		&compositeAlpha);

	if (result == false)
		return false;

	VkSwapchainKHR oldHandle = swapchain->handle;

	VkSwapchainKHR handle = createVkSwapchainHandle(
		surface,
		device,
		imageCount,
		surfaceFormat,
		surfaceExtent,
		surfaceTransform,
		compositeAlpha,
		presentMode,
		oldHandle);

	if (handle == NULL)
		return false;

	vkDestroySwapchainKHR(
		device,
		oldHandle,
		NULL);
	swapchain->handle = handle;

	VkFormat depthFormat;

	result = getBestVkDepthFormat(
		physicalDevice,
		useStencilBuffer,
		&depthFormat);

	if (result == false)
		return false;

	Vec3U imageSize = vec3U(
		surfaceExtent.width,
		surfaceExtent.height,
		1);

	// TODO: possibly optimize with fully dedicated memory block
	Image depthImage = createVkImage(
		allocator,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		depthFormat,
		window,
		IMAGE_2D_TYPE,
		IMAGE_FORMAT_COUNT,
		imageSize);

	if (depthImage == NULL)
		return false;

	VkImageView depthImageView = createVkDepthImageView(
		device,
		depthImage->vk.handle,
		depthFormat);

	if (depthImageView == NULL)
	{
		destroyVkImage(
			allocator,
			depthImage);
		return false;
	}

	VkRenderPass renderPass = createVkRenderPass(
		device,
		surfaceFormat.format,
		depthFormat);

	if (renderPass == NULL)
	{
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		destroyVkImage(
			allocator,
			depthImage);
		return false;
	}

	VkSwapchainBuffer* buffers;
	uint32_t bufferCount;

	result = createVkSwapchainBuffers(
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

	if (result == false)
	{
		vkDestroyRenderPass(
			device,
			renderPass,
			NULL);
		vkDestroyImageView(
			device,
			depthImageView,
			NULL);
		destroyVkImage(
			allocator,
			depthImage);
		return false;
	}

	swapchain->depthImage = depthImage;
	swapchain->depthImageView = depthImageView;
	swapchain->renderPass = renderPass;
	swapchain->buffers = buffers;
	swapchain->bufferCount = bufferCount;
	return true;
}
