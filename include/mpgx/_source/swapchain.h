#pragma once
#include "cmmt/common.h"
#include "mpgx/_source/image.h"

struct VkSwapchainFrame
{
	VkImage image;
	VkImageView imageView;
	VkFramebuffer framebuffer;
	VkCommandPool graphicsCommandPool;
	VkCommandPool presentCommandPool;
	VkCommandBuffer graphicsCommandBuffer;
	VkCommandBuffer presentCommandBuffer;
	VkDescriptorSet descriptorSet;
};

typedef struct VkSwapchainFrame* VkSwapchainFrame;

struct VkSwapchain
{
	VkSwapchainKHR handle;
	VkSwapchainFrame* frames;
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
		VkExtent2D extent = {
			clamp(framebufferSize.x,
				surfaceCapabilities->minImageExtent.width,
				surfaceCapabilities->maxImageExtent.width),
			clamp(framebufferSize.y,
				surfaceCapabilities->minImageExtent.height,
				surfaceCapabilities->maxImageExtent.height),
		};

		return extent;
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

inline static VkSwapchainKHR _createVkSwapchain(
	VkDevice device,
	VkSurfaceKHR surface,
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
inline static void _destroyVkSwapchain(
	VkDevice device,
	VkSwapchainKHR swapchain)
{
	vkDestroySwapchainKHR(
		device,
		swapchain,
		NULL);
}

inline static bool getBestVkDepthFormat(
	VkPhysicalDevice physicalDevice,
	bool useStencilBuffer,
	VkFormat* depthFormat)
{
	VkFormatProperties properties;

	if (useStencilBuffer == false)
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
		VK_FORMAT_D32_SFLOAT_S8_UINT,
		&properties);

	if(properties.optimalTilingFeatures &
	   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT)
	{
		*depthFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		return true;
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
	VkImageSubresourceRange subresourceRange = {
		VK_IMAGE_ASPECT_DEPTH_BIT,
		0,
		1,
		0,
		1,
	};
	VkImageViewCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		image,
		VK_IMAGE_VIEW_TYPE_2D,
		format,
		{0, 0, 0, 0},
		subresourceRange,
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
inline static void destroyVkImageView(
	VkDevice device,
	VkImageView imageView)
{
	vkDestroyImageView(
		device,
		imageView,
		NULL);
}
/* vk::RenderPass VkGpuSwapchain::createRenderPass(
	vk::Device device,
	vk::Format colorFormat,
	vk::Format depthFormat)
{
	vk::RenderPass renderPass;

	vk::AttachmentDescription attachmentDescriptions[2] =
		{
			vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				colorFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eStore,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::ePresentSrcKHR),
			vk::AttachmentDescription(
				vk::AttachmentDescriptionFlags(),
				depthFormat,
				vk::SampleCountFlagBits::e1,
				vk::AttachmentLoadOp::eClear,
				vk::AttachmentStoreOp::eDontCare,
				vk::AttachmentLoadOp::eDontCare,
				vk::AttachmentStoreOp::eDontCare,
				vk::ImageLayout::eUndefined,
				vk::ImageLayout::eDepthStencilAttachmentOptimal),
		};

	auto colorAttachmentReference = vk::AttachmentReference(
		0,
		vk::ImageLayout::eColorAttachmentOptimal);
	auto depthAttachmentReference = vk::AttachmentReference(
		1,
		vk::ImageLayout::eDepthStencilAttachmentOptimal);

	auto subpassDescription = vk::SubpassDescription(
		vk::SubpassDescriptionFlags(),
		vk::PipelineBindPoint::eGraphics,
		0,
		nullptr,
		1,
		&colorAttachmentReference,
		nullptr,
		&depthAttachmentReference,
		0,
		nullptr);

	auto subpassDependency = vk::SubpassDependency(
		VK_SUBPASS_EXTERNAL,
		0,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::PipelineStageFlagBits::eColorAttachmentOutput,
		vk::AccessFlags(),
		vk::AccessFlagBits::eColorAttachmentWrite,
		vk::DependencyFlags());

	auto renderPassCreateInfo = vk::RenderPassCreateInfo(
		vk::RenderPassCreateFlags(),
		2,
		attachmentDescriptions,
		1,
		&subpassDescription,
		1,
		&subpassDependency);

	auto result = device.createRenderPass(
		&renderPassCreateInfo,
		nullptr,
		&renderPass);

	if (result != vk::Result::eSuccess)
	{
		throw Exception(
			THIS_FUNCTION_NAME,
		"Failed to create render pass");
	}

	return renderPass;
}

std::vector<std::shared_ptr<VkSwapchainData>> VkGpuSwapchain::createDatas(
		vk::Device device,
		vk::SwapchainKHR swapchain,
		vk::RenderPass renderPass,
		vk::CommandPool graphicsCommandPool,
		vk::CommandPool presentCommandPool,
		vk::Format format,
		vk::ImageView depthImageView,
		const vk::Extent2D& extent)
	{
		auto images = device.getSwapchainImagesKHR(swapchain);
		auto datas = std::vector<std::shared_ptr<VkSwapchainData>>(images.size());

		for (size_t i = 0; i < images.size(); i++)
		{
			datas[i] = std::make_shared<VkSwapchainData>(
				device,
				images[i],
				renderPass,
				graphicsCommandPool,
				presentCommandPool,
				format,
				depthImageView,
				extent);
		}

		return std::move(datas);
	}
 */

inline static VkSwapchain createVkSwapchain(
	Window window,
	VkPhysicalDevice physicalDevice,
	VkSurfaceKHR surface,
	VkDevice device,
	VmaAllocator vmaAllocator,
	VkCommandPool graphicsCommandPool,
	VkCommandPool presentCommandPool,
	bool useStencilBuffer,
	Vec2U framebufferSize,
	VkSwapchainKHR oldSwapchain)
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

	VkSwapchainKHR handle = _createVkSwapchain(
		device,
		surface,
		imageCount,
		surfaceFormat,
		surfaceExtent,
		surfaceTransform,
		compositeAlpha,
		presentMode,
		oldSwapchain);

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
		_destroyVkSwapchain(device, handle);
		free(swapchain);
		return NULL;
	}

	Image depthImage = createVkImage(
		vmaAllocator,
		VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
		depthFormat,
		window,
		IMAGE_2D_TYPE,
		IMAGE_FORMAT_COUNT,
		vec3U(
			surfaceExtent.width,
			surfaceExtent.height,
			1));

	if (depthImage == NULL)
	{
		_destroyVkSwapchain(device, handle);
		free(swapchain);
		return NULL;
	}

	VkImageView depthImageView = createVkDepthImageView(
		device,
		depthImage->vk.handle,
		depthFormat);

	if (depthImageView == NULL)
	{
		destroyVkImage(vmaAllocator, depthImage);
		_destroyVkSwapchain(device, handle);
		free(swapchain);
		return NULL;
	}

	/*
	 renderPass = createRenderPass(
			_device,
			surfaceFormat.format,
			vkDepthFormat);

		datas = createDatas(
			_device,
			swapchain,
			renderPass,
			graphicsCommandPool,
			presentCommandPool,
			surfaceFormat.format,
			depthImageView,
			extent);
	 */

	swapchain->handle = handle;
	return swapchain;
}
inline static void destroyVkSwapchain(
	VkDevice device,
	VkSwapchain swapchain)
{
	if (swapchain == NULL)
		return;

	_destroyVkSwapchain(
		device,
		swapchain->handle);
	free(swapchain);
}
