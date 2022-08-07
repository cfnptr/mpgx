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
#include "mpgx/_source/buffer.h"
#include <assert.h>

typedef struct BaseImage_T
{
	Window window;
	Vec3I size;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	bool isConstant;
	uint32_t mipCount;
	uint32_t layerCount;
} BaseImage_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkImage_T
{
	Window window;
	Vec3I size;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	bool isConstant;
	uint32_t mipCount;
	uint32_t layerCount;
	VkFormat vkFormat;
	VkImageAspectFlagBits vkAspect;
	VkImage handle;
	VmaAllocation allocation;
	VkImageView imageView;
	VkBuffer stagingBuffer;
	VmaAllocation stagingAllocation;
	uint8_t sizeMultiplier;
} VkImage_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlImage_T
{
	Window window;
	Vec3I size;
	ImageType type;
	ImageDimension dimension;
	ImageFormat format;
	bool isConstant;
	uint32_t mipCount;
	uint32_t layerCount;
	GLenum glType;
	GLenum dataType;
	GLenum dataFormat;
	GLuint handle;
} GlImage_T;
#endif
union Image_T
{
	BaseImage_T base;
#if MPGX_SUPPORT_VULKAN
	VkImage_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlImage_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkImage(
	VkDevice device,
	VmaAllocator allocator,
	Image image)
{
	assert(device);
	assert(allocator);

	if (!image)
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
inline static MpgxResult fillVkImage(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	VkBuffer stagingBuffer,
	VmaAllocation stagingAllocation,
	const void** data,
	Vec3I size,
	uint32_t mipCount,
	uint32_t layerCount,
	VkImageAspectFlags vkAspect,
	VkDeviceSize bufferSize,
	uint8_t sizeMultiplier,
	VkImage handle)
{
	assert(device);
	assert(allocator);
	assert(transferQueue);
	assert(transferCommandBuffer);
	assert(transferFence);
	assert(stagingBuffer);
	assert(stagingAllocation);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(mipCount > 0);
	assert(layerCount > 0);
	assert(bufferSize > 0);
	assert(sizeMultiplier > 0);
	assert(handle);
	assert(mipCount <= calcMipLevelCount(size));

	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL,
	};

	VkResult vkResult = vkBeginCommandBuffer(
		transferCommandBuffer,
		&commandBufferBeginInfo);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

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
			mipCount,
			0,
			layerCount,
		},
	};

	vkCmdPipelineBarrier(
		transferCommandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	void* mapData;

	MpgxResult mpgxResult = mapVkBuffer(
		allocator,
		stagingAllocation,
		CPU_ONLY_BUFFER_USAGE,
		bufferSize,
		0,
		&mapData);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vkEndCommandBuffer(transferCommandBuffer);
		return mpgxResult;
	}

	uint8_t* map = (uint8_t*)mapData;
	Vec3I mipSize = size;
	size_t mipBufferSize = 0;

	for (uint32_t i = 0; i < mipCount; i++)
	{
		const uint8_t* array = (const uint8_t*)data[i];

		size_t copySize = (size_t)
			mipSize.x * mipSize.y * mipSize.z *
			layerCount * sizeMultiplier;

		if (array)
		{
			memcpy(map + mipBufferSize, array, copySize);

			VkBufferImageCopy bufferImageCopy = {
				mipBufferSize,
				0,
				0,
				{
					vkAspect,
					i,
					0,
					layerCount,
				},
				{
					0, 0, 0,
				},
				{
					mipSize.x,
					mipSize.y,
					mipSize.z
				}
			};

			vkCmdCopyBufferToImage(
				transferCommandBuffer,
				stagingBuffer,
				handle,
				VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
				1,
				&bufferImageCopy);
		}

		mipBufferSize += copySize;
		if (mipSize.x > 1) mipSize.x /= 2;
		if (mipSize.y > 1) mipSize.y /= 2;
		if (mipSize.z > 1) mipSize.z /= 2;
	}

	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE_KHR;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(
		transferCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	mpgxResult = unmapVkBuffer(
		allocator,
		stagingAllocation,
		CPU_ONLY_BUFFER_USAGE,
		bufferSize,
		0);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vkEndCommandBuffer(transferCommandBuffer);
		return mpgxResult;
	}

	mpgxResult = endSubmitWaitVkCommandBuffer(
		device,
		transferQueue,
		transferFence,
		transferCommandBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkImage(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	VkBuffer* stagingBuffer,
	VmaAllocation* stagingAllocation,
	size_t* stagingSize,
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3I size,
	uint32_t mipCount,
	uint32_t layerCount,
	bool isConstant,
	Image* image)
{
	assert(device);
	assert(allocator);
	assert(transferQueue);
	assert(transferCommandBuffer);
	assert(transferFence);
	assert(stagingBuffer);
	assert(stagingAllocation);
	assert(stagingSize);
	assert(window);
	assert(type > 0);
	assert(dimension < IMAGE_DIMENSION_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(mipCount > 0);
	assert(layerCount > 0);
	assert(mipCount <= calcMipLevelCount(size));
	assert(image);

	Image imageInstance = calloc(1, sizeof(Image_T));

	if (!imageInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	imageInstance->vk.window = window;
	imageInstance->vk.size = size;
	imageInstance->vk.type = type;
	imageInstance->vk.dimension = dimension;
	imageInstance->vk.format = format;
	imageInstance->vk.isConstant = isConstant;
	imageInstance->vk.mipCount = mipCount;
	imageInstance->vk.layerCount = layerCount;

	VkImageType vkType;
	VkImageViewType vkViewType;

	if (dimension == IMAGE_1D)
	{
		vkType = VK_IMAGE_TYPE_1D;
		vkViewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_1D_ARRAY : VK_IMAGE_VIEW_TYPE_1D;
	}
	else if (dimension == IMAGE_2D)
	{
		vkType = VK_IMAGE_TYPE_2D;
		vkViewType = layerCount > 1 ? VK_IMAGE_VIEW_TYPE_2D_ARRAY : VK_IMAGE_VIEW_TYPE_2D;
	}
	else if (dimension == IMAGE_3D)
	{
		vkType = VK_IMAGE_TYPE_3D;
		vkViewType = VK_IMAGE_VIEW_TYPE_3D;
	}
	else
	{
		destroyVkImage(
			device,
			allocator,
			imageInstance);
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkFormat vkFormat;
	VkImageAspectFlags vkAspect;
	uint8_t sizeMultiplier;

	VkImageUsageFlagBits vkUsage = 0;

	if (type & SAMPLED_IMAGE_TYPE)
		vkUsage |= VK_IMAGE_USAGE_SAMPLED_BIT;
	if (type & COLOR_ATTACHMENT_IMAGE_TYPE)
		vkUsage |= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
	if (type & DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE)
		vkUsage |= VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
	if (type & STORAGE_IMAGE_TYPE)
		vkUsage |= VK_IMAGE_USAGE_STORAGE_BIT;
	if (type & TRANSFER_SOURCE_IMAGE_TYPE)
		vkUsage |= VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
	if ((type & TRANSFER_DESTINATION_IMAGE_TYPE) || data)
		vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

	switch (format)
	{
	default:
		destroyVkImage(
			device,
			allocator,
			imageInstance);
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
	case R8_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8_UNORM;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 1;
		break;
	case R8_SRGB_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8_SRGB;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 1;
		break;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 4;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 4;
		break;
	case R16G16B16A16_SFLOAT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R16G16B16A16_SFLOAT;
		vkAspect = VK_IMAGE_ASPECT_COLOR_BIT;
		sizeMultiplier = 8;
		break;
	case D16_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D16_UNORM;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 2;
		break;
	case D32_SFLOAT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 4;
		break;
	case D16_UNORM_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D16_UNORM_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 3; // TODO: correct?
		break;
	case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 4;
		break;
	case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		vkAspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		sizeMultiplier = 5; // TODO: correct?
		break;
	}

	imageInstance->vk.vkFormat = vkFormat;
	imageInstance->vk.vkAspect = vkAspect;
	imageInstance->vk.sizeMultiplier = sizeMultiplier;

	VkImageCreateInfo imageCreateInfo = {
		VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
		NULL,
		0,
		vkType,
		vkFormat,
		{ size.x, size.y, size.z, },
		mipCount,
		layerCount,
		VK_SAMPLE_COUNT_1_BIT,
		isConstant ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
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

	if (type & COLOR_ATTACHMENT_IMAGE_TYPE ||
		type & DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE)
	{
		allocationCreateInfo.flags |= VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT;
	}

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
			imageInstance);
		return vkToMpgxResult(vkResult);
	}

	imageInstance->vk.handle = handle;
	imageInstance->vk.allocation = allocation;

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
			mipCount,
			0,
			layerCount,
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
			imageInstance);
		return vkToMpgxResult(vkResult);
	}

	imageInstance->vk.imageView = imageView;

	VkDeviceSize bufferSize = 0;
	Vec3I mipSize = size;

	for (uint32_t i = 0; i < mipCount; i++)
	{
		bufferSize += (VkDeviceSize)
			mipSize.x * mipSize.y * mipSize.z *
			layerCount * sizeMultiplier;
		if (mipSize.x > 1) mipSize.x /= 2;
		if (mipSize.y > 1) mipSize.y /= 2;
		if (mipSize.z > 1) mipSize.z /= 2;
	}

	VkBuffer stagingBufferInstance;
	VmaAllocation stagingAllocationInstance;

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
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;

	if (isConstant)
	{
		if (bufferSize > *stagingSize)
		{
			vkResult = vmaCreateBuffer(
				allocator,
				&bufferCreateInfo,
				&allocationCreateInfo,
				&stagingBufferInstance,
				&stagingAllocationInstance,
				NULL);

			if (vkResult != VK_SUCCESS)
			{
				destroyVkImage(
					device,
					allocator,
					imageInstance);
				return vkToMpgxResult(vkResult);
			}

			vmaDestroyBuffer(
				allocator,
				*stagingBuffer,
				*stagingAllocation);

			*stagingBuffer = stagingBufferInstance;
			*stagingAllocation = stagingAllocationInstance;
			*stagingSize = bufferSize;
		}
		else
		{
			stagingBufferInstance = *stagingBuffer;
			stagingAllocationInstance = *stagingAllocation;
		}

		imageInstance->vk.stagingBuffer = NULL;
		imageInstance->vk.stagingAllocation = NULL;
	}
	else
	{
		vkResult = vmaCreateBuffer(
			allocator,
			&bufferCreateInfo,
			&allocationCreateInfo,
			&stagingBufferInstance,
			&stagingAllocationInstance,
			NULL);

		if (vkResult != VK_SUCCESS)
		{
			destroyVkImage(
				device,
				allocator,
				imageInstance);
			return vkToMpgxResult(vkResult);
		}

		imageInstance->vk.stagingBuffer = stagingBufferInstance;
		imageInstance->vk.stagingAllocation = stagingAllocationInstance;
	}

	if (data)
	{
		MpgxResult mpgxResult = fillVkImage(
			device,
			allocator,
			transferQueue,
			transferCommandBuffer,
			transferFence,
			stagingBufferInstance,
			stagingAllocationInstance,
			data,
			size,
			mipCount,
			layerCount,
			vkAspect,
			bufferSize,
			sizeMultiplier,
			handle);

		if (mpgxResult != SUCCESS_MPGX_RESULT)
		{
			destroyVkImage(
				device,
				allocator,
				imageInstance);
			return mpgxResult;
		}
	}

	*image = imageInstance;
	return SUCCESS_MPGX_RESULT;
}

// TODO: add separated array layer setter
inline static MpgxResult setVkImageData(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	Image image,
	const void* data,
	Vec3I size,
	Vec3I offset,
	uint8_t mipLevel)
{
	assert(device);
	assert(allocator);
	assert(transferQueue);
	assert(transferCommandBuffer);
	assert(transferFence);
	assert(image);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(offset.x >= 0);
	assert(offset.y >= 0);
	assert(offset.z >= 0);
	assert(mipLevel < image->vk.mipCount);

	// TODO: properly add staging buffer image data offset
	assert(offset.x == 0 && offset.y == 0 && offset.z == 0);

	size_t dataSize = (size_t)size.x * size.y * size.z *
		image->vk.layerCount * image->vk.sizeMultiplier;

	MpgxResult mpgxResult = setVkBufferData(
		allocator,
		image->vk.stagingAllocation,
		data,
		dataSize,
		0);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL,
	};

	VkResult vkResult = vkBeginCommandBuffer(
		transferCommandBuffer,
		&commandBufferBeginInfo);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	VkImage handle = image->vk.handle;
	VkImageAspectFlagBits aspect = image->vk.vkAspect;
	uint32_t mipCount = image->vk.mipCount;
	uint32_t layerCount = image->vk.layerCount;

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
			aspect,
			mipLevel, // TODO: correct?
			1,
			0,
			layerCount,
		},
	};

	vkCmdPipelineBarrier(
		transferCommandBuffer,
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
			mipLevel,
			0,
			layerCount,
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
		transferCommandBuffer,
		image->vk.stagingBuffer,
		handle,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&bufferImageCopy);

	imageMemoryBarrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
	imageMemoryBarrier.dstAccessMask = VK_ACCESS_NONE;
	imageMemoryBarrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
	imageMemoryBarrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;

	vkCmdPipelineBarrier(
		transferCommandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	mpgxResult = endSubmitWaitVkCommandBuffer(
		device,
		transferQueue,
		transferFence,
		transferCommandBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static void destroyGlImage(
	Image image)
{
	if (!image)
		return;

	makeGlWindowContextCurrent(
		image->gl.window);

	glDeleteTextures(
		GL_ONE,
		&image->gl.handle);
	assertOpenGL();

	free(image);
}
inline static MpgxResult createGlImage(
	Window window,
	ImageType type,
	ImageDimension dimension,
	ImageFormat format,
	const void** data,
	Vec3I size,
	uint32_t mipCount,
	bool isConstant,
	Image* image)
{
	assert(window);
	assert(type > 0);
	assert(dimension < IMAGE_DIMENSION_COUNT);
	assert(format < IMAGE_FORMAT_COUNT);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(mipCount > 0);
	assert(mipCount <= calcMipLevelCount(size));
	assert(image);

	if (!(type & SAMPLED_IMAGE_TYPE) &&
		!(type & COLOR_ATTACHMENT_IMAGE_TYPE) &&
		!(type & DEPTH_STENCIL_ATTACHMENT_IMAGE_TYPE))
	{
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	// TODO: use isAttachment for renderbuffer optimization

	Image imageInstance = calloc(1, sizeof(Image_T));

	if (!imageInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	imageInstance->gl.window = window;
	imageInstance->gl.size = size;
	imageInstance->gl.type = type;
	imageInstance->gl.dimension = dimension;
	imageInstance->gl.format = format;
	imageInstance->gl.isConstant = isConstant;
	imageInstance->gl.mipCount = mipCount;
	imageInstance->gl.layerCount = 1;

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
		destroyGlImage(imageInstance);
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	imageInstance->gl.glType = glType;

	GLint glFormat;
	GLenum dataFormat;
	GLenum dataType;

	switch (format)
	{
	default:
		destroyGlImage(imageInstance);
		return FORMAT_IS_NOT_SUPPORTED_MPGX_RESULT;
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

	imageInstance->gl.dataType = dataType;
	imageInstance->gl.dataFormat = dataFormat;

	makeGlWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenTextures(
		GL_ONE,
		&handle);

	imageInstance->gl.handle = handle;

	glBindTexture(
		glType,
		handle);

	if (dimension == IMAGE_2D)
	{
		Vec2I mipSize = vec2I(size.x, size.y);

		for (uint32_t i = 0; i < mipCount; i++)
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
				data ? data[i] : NULL);

			mipSize = divValVec2I(
				mipSize, 2);
		}

		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_BASE_LEVEL,
			0);
		glTexParameteri(
			GL_TEXTURE_2D,
			GL_TEXTURE_MAX_LEVEL,
			(GLint)(mipCount - 1));
	}
	else
	{
		Vec3I mipSize = size;

		for (uint32_t i = 0; i < mipCount; i++)
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
				data ? data[i] : NULL);
			mipSize = divValVec3I(
				mipSize, 2);
		}

		glTexParameteri(
			GL_TEXTURE_3D,
			GL_TEXTURE_BASE_LEVEL,
			0);
		glTexParameteri(
			GL_TEXTURE_3D,
			GL_TEXTURE_MAX_LEVEL,
			(GLint)(mipCount - 1));
	}

	GLenum glError = glGetError();

	if (glError != GL_NO_ERROR)
	{
		destroyGlImage(imageInstance);
		return glToMpgxResult(glError);
	}

	*image = imageInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult setGlImageData(
	Image image,
	const void* data,
	Vec3I size,
	Vec3I offset,
	uint8_t mipLevel)
{
	assert(image);
	assert(data);
	assert(size.x > 0);
	assert(size.y > 0);
	assert(size.z > 0);
	assert(offset.x >= 0);
	assert(offset.y >= 0);
	assert(offset.z >= 0);
	assert(mipLevel < image->gl.mipCount);

	makeGlWindowContextCurrent(
		image->gl.window);

	glBindTexture(
		image->gl.glType,
		image->gl.handle);

	ImageDimension dimension = image->gl.dimension;

	if (dimension == IMAGE_2D)
	{
		glTexSubImage2D(
			image->gl.glType,
			(GLint)mipLevel,
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
			(GLint)mipLevel,
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

	GLenum glError = glGetError();

	if (glError != GL_NO_ERROR)
		return glToMpgxResult(glError);

	return SUCCESS_MPGX_RESULT;
}
#endif
