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
#include "mpgx/_source/buffer.h"
#include <assert.h>

// TODO: handle Vulkan unsupported formats on platforms
// VkGetPhysicalDeviceImageFormatProperties
// https://stackoverflow.com/questions/38396578/vulkan-vkcreateimage-with-3-components

typedef struct BaseImage_T
{
	Window window;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	Vec3U size;
	bool isConstant;
} BaseImage_T;
typedef struct VkImage_T
{
	Window window;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	Vec3U size;
	bool isConstant;
#if MPGX_SUPPORT_VULKAN
	VkFormat vkFormat;
	VkImageAspectFlagBits vkAspect;
	uint8_t sizeMultiplier;
	VkImage handle;
	VmaAllocation allocation;
	VkImageView imageView;
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
#endif
} VkImage_T;
typedef struct GlImage_T
{
	Window window;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	Vec3U size;
	bool isConstant;
	GLenum glType;
	GLenum dataType;
	GLenum dataFormat;
	GLuint handle;
} GlImage_T;
union Image_T
{
	BaseImage_T base;
	VkImage_T vk;
	GlImage_T gl;
};

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkImage(
	VkDevice device,
	VmaAllocator allocator,
	Image image)
{
	if (image == NULL)
		return;

	vmaDestroyBuffer(
		allocator,
		image->vk.stagingBuffer,
		image->vk.stagingAllocation);
	vkDestroyImageView(
		device,
		image->vk.imageView,
		NULL);
	vmaDestroyImage(
		allocator,
		image->vk.handle,
		image->vk.allocation);
	free(image);
}
inline static Image createVkImage(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	VkBuffer* _stagingBuffer,
	VmaAllocation* _stagingAllocation,
	size_t* _stagingSize,
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant)
{
	// TODO: mipmap generation, multisampling

	Image image = calloc(1, sizeof(Image_T));

	if (image == NULL)
		return NULL;

	image->vk.window = window;
	image->vk.type = type;
	image->vk.dimension = dimension;
	image->vk.format = format;
	image->vk.size = size;
	image->vk.isConstant = isConstant;

	VkImageType vkType;
	VkImageViewType vkViewType;

	if (dimension == IMAGE_1D)
	{
		vkType = VK_IMAGE_TYPE_1D;
		vkViewType = VK_IMAGE_VIEW_TYPE_1D;
	}
	else if (dimension == IMAGE_2D)
	{
		vkType = VK_IMAGE_TYPE_2D;
		vkViewType = VK_IMAGE_VIEW_TYPE_2D;
	}
	else if (dimension == IMAGE_3D)
	{
		vkType = VK_IMAGE_TYPE_3D;
		vkViewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else
	{
		abort();
	}

	VkFormat vkFormat;
	VkImageAspectFlags vkAspect;
	uint8_t sizeMultiplier;

	VkImageUsageFlagBits vkUsage = VK_IMAGE_USAGE_SAMPLED_BIT;

	if (type == STORAGE_IMAGE_TYPE)
	{
		vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT |
			VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	}

	switch (format)
	{
	default:
		destroyVkImage(
			device,
			allocator,
			image);
		return NULL;
	case R8_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8_UNORM;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 1;

		if (type == true)
			vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 4;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 4;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case R16G16B16A16_SFLOAT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 8;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
		break;
	case D16_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D16_UNORM;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 2;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	case D32_SFLOAT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 4;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	case D16_UNORM_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D16_UNORM_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 3; // TODO: correct?

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 4;

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 5; // TODO: correct?

		if (type == ATTACHMENT_IMAGE_TYPE)
			vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		break;
	}

	if (data[0] != NULL)
		vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	image->vk.vkFormat = vkFormat;
	image->vk.vkAspect = vkAspect;
	image->vk.sizeMultiplier = sizeMultiplier;

	VkImageCreateInfo imageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		vkType,
		vkFormat,
		{ size.x, size.y, size.z, },
		1,
		1,
		VK_SAMPLE_COUNT_1_BIT,
		isConstant == true ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
		vkUsage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocationCreateInfo;
	memset(&allocationCreateInfo, 0, sizeof(VmaAllocationCreateInfo));

	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

	if (type == ATTACHMENT_IMAGE_TYPE)
		allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;

	VkImage handle;
	VmaAllocation allocation;

	VkResult vkResult = vmaCreateImage(
		allocator,
		&imageCreateInfo,
		&allocationCreateInfo,
		&handle,
		&allocation,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkImage(
			device,
			allocator,
			image);
		return NULL;
	}

	image->vk.handle = handle;
	image->vk.allocation = allocation;

	VkImageViewCreateInfo imageViewCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
		NULL,
		0,
		handle,
		vkViewType,
		vkFormat,
		{
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
			VK_COMPONENT_SWIZZLE_IDENTITY,
		},
		{
			vkAspect,
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
		destroyVkImage(
			device,
			allocator,
			image);
		return NULL;
	}

	image->vk.imageView = imageView;

	VkDeviceSize bufferSize =
		size.x * size.y * size.z *
		sizeMultiplier;

	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;

	VkBufferCreateInfo bufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		bufferSize,
		VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
	};

	allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

	if (isConstant == false)
	{
		vkResult = vmaCreateBuffer(
			allocator,
			&bufferCreateInfo,
			&allocationCreateInfo,
			&stagingBuffer,
			&stagingAllocation,
			NULL);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkImage(
				device,
				allocator,
				image);
			return NULL;
		}

		image->vk.stagingBuffer = stagingBuffer;
		image->vk.stagingAllocation = stagingAllocation;
	}
	else
	{
		if (bufferSize > *_stagingSize)
		{
			vkResult = vmaCreateBuffer(
				allocator,
				&bufferCreateInfo,
				&allocationCreateInfo,
				&stagingBuffer,
				&stagingAllocation,
				NULL);

			if (vkResult != VK_SUCCESS)
			{
				destroyVkImage(
					device,
					allocator,
					image);
				return NULL;
			}

			vmaDestroyBuffer(
				allocator,
				*_stagingBuffer,
				*_stagingAllocation);

			*_stagingBuffer = stagingBuffer;
			*_stagingAllocation = stagingAllocation;
			*_stagingSize = bufferSize;
		}
		else
		{
			stagingBuffer = *_stagingBuffer;
			stagingAllocation = *_stagingAllocation;
		}

		image->vk.stagingBuffer = NULL;
		image->vk.stagingAllocation = NULL;
	}

	if (data[0] != NULL)
	{
		setVkBufferData(
			allocator,
			stagingAllocation,
			data[0],
			bufferSize,
			0);

		VkCommandBuffer commandBuffer = allocateBeginVkOneTimeCommandBuffer(
			device, transferCommandPool);

		if (commandBuffer == NULL)
		{
			destroyVkImage(
				device,
				allocator,
				image);
			return NULL;
		}

		VkImageMemoryBarrier imageMemoryBarrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			VK_ACCESS_NONE_KHR,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			handle,
			{
				vkAspect,
				0,
				1,
				0,
				1,
			},
		};

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			0,
			0,
			NULL,
			0,
			NULL,
			1,
			&imageMemoryBarrier);

		VkBufferImageCopy bufferImageCopy = {
			0,
			0,
			0,
			{
				vkAspect,
				0,
				0,
				1,
			},
			{
				0, 0, 0,
			},
			{
				size.x, size.y, size.z
			}
		};

		vkCmdCopyBufferToImage(
			commandBuffer,
			stagingBuffer,
			handle,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			1,
			&bufferImageCopy);

		imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
		imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
		imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
		imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TRANSFER_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			NULL,
			0,
			NULL,
			1,
			&imageMemoryBarrier);

		bool result = endSubmitWaitFreeVkCommandBuffer(
			device,
			transferQueue,
			transferCommandPool,
			transferFence,
			commandBuffer);

		if (result == false)
		{
			destroyVkImage(
				device,
				allocator,
				image);
			return NULL;
		}
	}
	else
	{
		VkCommandBuffer commandBuffer = allocateBeginVkOneTimeCommandBuffer(
			device, transferCommandPool);

		if (commandBuffer == NULL)
		{
			destroyVkImage(
				device,
				allocator,
				image);
			return NULL;
		}

		VkImageMemoryBarrier imageMemoryBarrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			VK_ACCESS_NONE_KHR,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			handle,
			{
				vkAspect,
				0,
				1,
				0,
				1,
			},
		};

		vkCmdPipelineBarrier(
			commandBuffer,
			VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			NULL,
			0,
			NULL,
			1,
			&imageMemoryBarrier);

		bool result = endSubmitWaitFreeVkCommandBuffer(
			device,
			transferQueue,
			transferCommandPool,
			transferFence,
			commandBuffer);

		if (result == false)
		{
			destroyVkImage(
				device,
				allocator,
				image);
			return NULL;
		}
	}

	return image;
}

inline static bool setVkImageData(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	VkBuffer stagingBuffer,
	VmaAllocation stagingAllocation,
	VkImage image,
	VkImageAspectFlags aspect,
	uint8_t sizeMultiplier,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	// TODO: properly add staging buffer image data offset
	assert(offset.x == 0 && offset.y == 0 && offset.z == 0);

	size_t dataSize =
		size.x * size.y * size.z *
		sizeMultiplier;

	setVkBufferData(
		allocator,
		stagingAllocation,
		data,
		dataSize,
		0);

	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		transferCommandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	VkCommandBuffer commandBuffer = allocateBeginVkOneTimeCommandBuffer(
		device, transferCommandPool);

	if (commandBuffer == NULL)
		return false;

	VkImageMemoryBarrier imageMemoryBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_NONE_KHR,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		image,
		{
			aspect,
			0,
			1,
			0,
			1,
		},
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	VkBufferImageCopy bufferImageCopy = {
		0,
		0,
		0,
		{
			aspect,
			0,
			0,
			1,
		},
		{
			(int32_t)offset.x,
			(int32_t)offset.y,
			(int32_t)offset.z,
		},
		{
			size.x, size.y, size.z
		}
	};

	vkCmdCopyBufferToImage(
		commandBuffer,
		stagingBuffer,
		image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferImageCopy);

	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	bool result = endSubmitWaitFreeVkCommandBuffer(
		device,
		transferQueue,
		transferCommandPool,
		transferFence,
		commandBuffer);

	if (result == false)
		return false;

	return true;
}
#endif

inline static void destroyGlImage(
	Image image)
{
	if (image == NULL)
		return;

	makeWindowContextCurrent(
		image->gl.window);

	glDeleteTextures(
		GL_ONE,
		&image->gl.handle);
	assertOpenGL();

	free(image);
}
inline static Image createGlImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant)
{
	// TODO: use isAttachment for renderbuffer optimization

	Image image = calloc(1, sizeof(Image_T));

	if (image == NULL)
		return NULL;

	image->gl.window = window;
	image->gl.type = type;
	image->gl.dimension = dimension;
	image->gl.format = format;
	image->gl.size = size;
	image->gl.isConstant = isConstant;

	GLenum glType;

	if (dimension == IMAGE_2D)
	{
		glType = GL_TEXTURE_2D;
	}
	else if (dimension == IMAGE_3D)
	{
		glType = GL_TEXTURE_3D;
	}
	else
	{
		destroyGlImage(image);
		return NULL;
	}

	image->gl.glType = glType;

	GLint glFormat;
	GLenum dataFormat;
	GLenum dataType;

	switch (format)
	{
	default:
		destroyGlImage(image);
		return NULL;
	case R8_UNORM_IMAGE_FORMAT:
		glFormat = GL_R8;
		dataFormat = GL_RED;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		glFormat = GL_RGBA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		glFormat = GL_SRGB8_ALPHA8;
		dataFormat = GL_RGBA;
		dataType = GL_UNSIGNED_BYTE;
		break;
	case R16G16B16A16_SFLOAT_IMAGE_FORMAT:
		glFormat = GL_RGBA16F;
		dataFormat = GL_RGBA;
		dataType = GL_FLOAT;
		break;
	case D16_UNORM_IMAGE_FORMAT:
		glFormat = GL_DEPTH_COMPONENT16;
		dataFormat = GL_DEPTH_COMPONENT;
		dataType = GL_UNSIGNED_SHORT;
		break;
	case D32_SFLOAT_IMAGE_FORMAT:
		glFormat = GL_DEPTH_COMPONENT32F;
		dataFormat = GL_DEPTH_COMPONENT;
		dataType = GL_FLOAT;
		break;
	case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		glFormat = GL_DEPTH24_STENCIL8;
		dataFormat = GL_DEPTH_STENCIL;
		dataType = GL_UNSIGNED_INT_24_8;
		break;
	case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
		glFormat = GL_DEPTH32F_STENCIL8;
		dataFormat = GL_DEPTH_STENCIL;
		dataType = GL_FLOAT_32_UNSIGNED_INT_24_8_REV;
		break;
	}

	image->gl.dataType = dataType;
	image->gl.dataFormat = dataFormat;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenTextures(
		GL_ONE,
		&handle);

	image->gl.handle = handle;

	glBindTexture(
		glType,
		handle);

	if (dimension == IMAGE_2D)
	{
		if (levelCount == 0)
		{
			glTexImage2D(
				glType,
				0,
				glFormat,
				(GLsizei)size.x,
				(GLsizei)size.y,
				0,
				dataFormat,
				dataType,
				data[0]);
			glGenerateMipmap(glType);
		}
		else
		{
			Vec2U mipSize = vec2U(size.x, size.y);

			for (uint8_t i = 0; i < levelCount; i++)
			{
				glTexImage2D(
					glType,
					(GLint)i,
					glFormat,
					(GLsizei)mipSize.x,
					(GLsizei)mipSize.y,
					0,
					dataFormat,
					dataType,
					data[i]);

				mipSize = vec2U(
					mipSize.x / 2,
					mipSize.y / 2);
			}

			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_BASE_LEVEL,
				0);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAX_LEVEL,
				levelCount - 1);
		}
	}
	else
	{
		if (levelCount == 0)
		{
			glTexImage3D(
				glType,
				0,
				glFormat,
				(GLsizei)size.x,
				(GLsizei)size.y,
				(GLsizei)size.z,
				0,
				dataFormat,
				dataType,
				data[0]);
			glGenerateMipmap(glType);
		}
		else
		{
			Vec3U mipSize = size;

			for (uint8_t i = 0; i < levelCount; i++)
			{
				glTexImage3D(
					glType,
					(GLint)i,
					glFormat,
					(GLsizei)mipSize.x,
					(GLsizei)mipSize.y,
					(GLsizei)mipSize.z,
					0,
					dataFormat,
					dataType,
					data[i]);

				mipSize = vec3U(
					mipSize.x / 2,
					mipSize.y / 2,
					mipSize.z / 2);
			}

			glTexParameteri(
				GL_TEXTURE_3D,
				GL_TEXTURE_BASE_LEVEL,
				0);
			glTexParameteri(
				GL_TEXTURE_3D,
				GL_TEXTURE_MAX_LEVEL,
				levelCount - 1);
		}
	}

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlImage(image);
		return NULL;
	}

	return image;
}

inline static void setGlImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	makeWindowContextCurrent(
		image->gl.window);

	glBindTexture(
		image->gl.glType,
		image->gl.handle);

	ImageDimension dimension = image->gl.dimension;

	if (dimension == IMAGE_2D)
	{
		glTexSubImage2D(
			image->gl.glType,
			0,
			(GLint)offset.x,
			(GLint)offset.y,
			(GLsizei)size.x,
			(GLsizei)size.y,
			image->gl.dataFormat,
			image->gl.dataType,
			data);
	}
	else if (dimension == IMAGE_3D)
	{
		glTexSubImage3D(
			image->gl.glType,
			0,
			(GLint)offset.x,
			(GLint)offset.y,
			(GLint)offset.z,
			(GLsizei)size.x,
			(GLsizei)size.y,
			(GLsizei)size.z,
			image->gl.dataFormat,
			image->gl.dataType,
			data);
	}
	else
	{
		abort();
	}

	assertOpenGL();
}
