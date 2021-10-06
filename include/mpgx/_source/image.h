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
#include "mpgx/_source/opengl.h"

#if MPGX_SUPPORT_VULKAN
#include "vk_mem_alloc.h"
#endif

#include <string.h>

// TODO: handle Vulkan unsupported formats on platforms
// VkGetPhysicalDeviceImageFormatProperties
// https://stackoverflow.com/questions/38396578/vulkan-vkcreateimage-with-3-components

typedef struct _VkImage
{
	Window window;
	ImageType type;
	ImageFormat format;
	Vec3U size;
#if MPGX_SUPPORT_VULKAN
	VkImage handle;
	VmaAllocation allocation;
#endif
} _VkImage;
typedef struct _GlImage
{
	Window window;
	ImageType type;
	ImageFormat format;
	Vec3U size;
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
	VmaAllocator allocator,
	VkImageUsageFlags _vkUsage,
	VkFormat _vkFormat,
	Window window,
	ImageType type,
	ImageFormat format,
	Vec3U size)
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
	VkImageUsageFlags vkUsage;

	if (_vkFormat == VK_FORMAT_UNDEFINED)
	{
		switch (format)
		{
		default:
			free(image);
			return NULL;
		case R8G8B8A8_UNORM_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_R8G8B8A8_UNORM;
			vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			break;
		case R8G8B8A8_SRGB_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_R8G8B8A8_SRGB;
			vkUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;
			break;
		case D16_UNORM_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_D16_UNORM;
			vkUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		case D32_SFLOAT_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_D32_SFLOAT;
			vkUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		case D24_UNORM_S8_UINT_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_D24_UNORM_S8_UINT;
			vkUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		case D32_SFLOAT_S8_UINT_IMAGE_FORMAT:
			vkFormat = VK_FORMAT_D32_SFLOAT_S8_UINT;
			vkUsage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
			break;
		}

		vkUsage |= _vkUsage;
	}
	else
	{
		vkFormat = _vkFormat;
		vkUsage = _vkUsage;
	}

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
		VK_IMAGE_TILING_OPTIMAL,
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

	VkResult result = vmaCreateImage(
		allocator,
		&imageCreateInfo,
		&allocationCreateInfo,
		&handle,
		&allocation,
		NULL);

	if (result != VK_SUCCESS)
	{
		free(image);
		return NULL;
	}

	image->vk.window = window;
	image->vk.type = type;
	image->vk.format = format;
	image->vk.size = size;
	image->vk.handle = handle;
	image->vk.handle = handle;
	image->vk.allocation = allocation;
	return image;
}
#endif

inline static Image createGlImage(
	Window window,
	ImageType type,
	ImageFormat format,
	Vec3U size,
	const void** data,
	uint8_t levelCount)
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
