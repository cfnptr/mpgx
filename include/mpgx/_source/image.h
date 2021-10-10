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

// TODO: handle Vulkan unsupported formats on platforms
// VkGetPhysicalDeviceImageFormatProperties
// https://stackoverflow.com/questions/38396578/vulkan-vkcreateimage-with-3-components

typedef struct _VkImage
{
	Window window;
	ImageType type;
	ImageFormat format;
	Vec3U size;
	bool isConstant;
#if MPGX_SUPPORT_VULKAN
	VkImage handle;
	VmaAllocation allocation;
	VkFormat vkFormat;
#endif
} _VkImage;
typedef struct _GlImage
{
	Window window;
	ImageType type;
	ImageFormat format;
	Vec3U size;
	bool isConstant;
	GLenum glType;
	GLenum dataType;
	GLenum dataFormat;
	GLuint handle;
} _GlImage;
union Image
{
	_VkImage vk;
	_GlImage gl;
};

#if MPGX_SUPPORT_VULKAN
inline static Image createVkImage(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkImageUsageFlags vkUsage,
	Window window,
	ImageType type,
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant)
{
	// TODO: mipmap generation, multisampling

	Image image = malloc(
		sizeof(union Image));

	if (image == NULL)
		return NULL;

	VkImageType vkType;

	if (type == IMAGE_1D_TYPE)
		vkType = VK_IMAGE_TYPE_1D;
	else if (type == IMAGE_2D_TYPE)
		vkType = VK_IMAGE_TYPE_2D;
	else if (type == IMAGE_3D_TYPE)
		vkType = VK_IMAGE_TYPE_3D;
	else
		abort();

	VkFormat vkFormat;
	VkImageAspectFlags aspect;
	size_t bufferSize;

	switch (format)
	{
	default:
		free(image);
		return NULL;
	case R8G8B8A8_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferSize = size.x * size.y * size.z * 4;
		break;
	case R8G8B8A8_SRGB_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
		aspect = VK_IMAGE_ASPECT_COLOR_BIT;
		bufferSize = size.x * size.y * size.z * 4;
		break;
	case D16_UNORM_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D16_UNORM;
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		bufferSize = size.x * size.y * size.z * 2;
		break;
	case D32_SFLOAT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT;
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		bufferSize = size.x * size.y * size.z * 4;
		break;
	case D24_UNORM_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		bufferSize = size.x * size.y * size.z * 4;
		break;
	case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
		vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
		aspect = VK_IMAGE_ASPECT_DEPTH_BIT;
		bufferSize = size.x * size.y * size.z * 5; // TODO: correct?
		break;
	}

	if (data[0] != NULL)
		vkUsage |= VK_IMAGE_USAGE_TRANSFER_DST_BIT;

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
		isConstant ? VK_IMAGE_TILING_OPTIMAL : VK_IMAGE_TILING_LINEAR,
		vkUsage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
		VK_IMAGE_LAYOUT_UNDEFINED,
	};

	VmaAllocationCreateInfo allocationCreateInfo;

	memset(
		&allocationCreateInfo,
		0,
		sizeof(VmaAllocationCreateInfo));

	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

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
		free(image);
		return NULL;
	}

	if (data[0] != NULL)
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			transferCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		VkCommandBuffer commandBuffer;

		vkResult = vkAllocateCommandBuffers(
			device,
			&commandBufferAllocateInfo,
			&commandBuffer);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

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

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		vkResult = vmaCreateBuffer(
			allocator,
			&bufferCreateInfo,
			&allocationCreateInfo,
			&stagingBuffer,
			&stagingAllocation,
			NULL);

		if (vkResult != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		bool result = setVkBufferData(
			allocator,
			stagingAllocation,
			data[0],
			bufferSize,
			0);

		if (result == false)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			NULL,
		};

		vkResult = vkBeginCommandBuffer(
			commandBuffer,
			&commandBufferBeginInfo);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkImageMemoryBarrier imageMemoryBarrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			0,
			VK_ACCESS_TRANSFER_WRITE_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			handle,
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

		// TODO: Transition/sync

		vkResult = vkEndCommandBuffer(commandBuffer);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkSubmitInfo submitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			NULL,
			0,
			NULL,
			NULL,
			1,
			&commandBuffer,
			0,
			NULL,
		};

		vkResult = vkQueueSubmit(
			transferQueue,
			1,
			&submitInfo,
			NULL);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		vkResult = vkQueueWaitIdle(transferQueue);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		vmaDestroyBuffer(
			allocator,
			stagingBuffer,
			stagingAllocation);
		vkFreeCommandBuffers(
			device,
			transferCommandPool,
			1,
			&commandBuffer);
	}
	else
	{
		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			transferCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		VkCommandBuffer commandBuffer;

		vkResult = vkAllocateCommandBuffers(
			device,
			&commandBufferAllocateInfo,
			&commandBuffer);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			NULL,
		};

		vkResult = vkBeginCommandBuffer(
			commandBuffer,
			&commandBufferBeginInfo);

		if (vkResult != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkImageMemoryBarrier imageMemoryBarrier = {
			VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
			NULL,
			0,
			VK_ACCESS_SHADER_READ_BIT,
			VK_IMAGE_LAYOUT_UNDEFINED,
			VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
			VK_QUEUE_FAMILY_IGNORED,
			VK_QUEUE_FAMILY_IGNORED,
			handle,
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
			VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
			0,
			0,
			NULL,
			0,
			NULL,
			1,
			&imageMemoryBarrier);

		vkResult = vkEndCommandBuffer(commandBuffer);

		if (vkResult != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		VkSubmitInfo submitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			NULL,
			0,
			NULL,
			NULL,
			1,
			&commandBuffer,
			0,
			NULL,
		};

		vkResult = vkQueueSubmit(
			transferQueue,
			1,
			&submitInfo,
			NULL);

		if (vkResult != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		vkResult = vkQueueWaitIdle(transferQueue);

		if (vkResult != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyImage(
				allocator,
				handle,
				allocation);
			free(image);
			return NULL;
		}

		vkFreeCommandBuffers(
			device,
			transferCommandPool,
			1,
			&commandBuffer);
	}

	image->vk.window = window;
	image->vk.type = type;
	image->vk.format = format;
	image->vk.size = size;
	image->vk.isConstant = isConstant;
	image->vk.handle = handle;
	image->vk.allocation = allocation;
	image->vk.vkFormat = vkFormat;
	return image;
}
#endif

inline static Image createGlImage(
	Window window,
	ImageType type,
	ImageFormat format,
	const void** data,
	Vec3U size,
	uint8_t levelCount,
	bool isConstant)
{
	Image image = malloc(
		sizeof(union Image));

	if (image == NULL)
		return NULL;

	GLenum glType;
	GLenum dataFormat;
	GLenum dataType;

	if (type == IMAGE_2D_TYPE)
	{
		glType = GL_TEXTURE_2D;
	}
	else if (type == IMAGE_3D_TYPE)
	{
		glType = GL_TEXTURE_3D;
	}
	else
	{
		free(image);
		return NULL;
	}

	GLint glFormat;

	switch (format)
	{
	default:
		free(image);
		return NULL;
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

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenTextures(
		GL_ONE,
		&handle);
	glBindTexture(
		glType,
		handle);

	if (type == IMAGE_2D_TYPE)
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
		glDeleteTextures(
			GL_ONE,
			&handle);
		free(image);
		return NULL;
	}

	image->gl.window = window;
	image->gl.type = type;
	image->gl.format = format;
	image->gl.size = size;
	image->gl.isConstant = isConstant;
	image->gl.glType = glType;
	image->gl.dataType = dataType;
	image->gl.dataFormat = dataFormat;
	image->gl.handle = handle;
	return image;
}

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkImage(
	VmaAllocator allocator,
	Image image)
{
	vmaDestroyImage(
		allocator,
		image->vk.handle,
		image->vk.allocation);
	free(image);
}
#endif

inline static void destroyGlImage(
	Image image)
{
	makeWindowContextCurrent(
		image->gl.window);

	glDeleteTextures(
		GL_ONE,
		&image->gl.handle);
	assertOpenGL();

	free(image);
}

#if MPGX_SUPPORT_VULKAN
inline static void setVkImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	// TODO:
}
#endif

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

	ImageType type = image->gl.type;

	if (type == IMAGE_2D_TYPE)
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
	else if (type == IMAGE_3D_TYPE)
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
