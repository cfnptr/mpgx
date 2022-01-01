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
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/opengl.h"

typedef struct BaseSampler_T
{
	Window window;
	ImageFilter minImageFilter;
	ImageFilter magImageFilter;
	ImageFilter minMipmapFilter;
	bool useMipmapping;
	ImageWrap imageWrapX;
	ImageWrap imageWrapY;
	ImageWrap imageWrapZ;
	CompareOperator compareOperator;
	bool useCompare;
	Vec2F mipmapLodRange;
	float mipmapLodBias;
} BaseSampler_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkSampler_T
{
	Window window;
	ImageFilter minImageFilter;
	ImageFilter magImageFilter;
	ImageFilter minMipmapFilter;
	bool useMipmapping;
	ImageWrap imageWrapX;
	ImageWrap imageWrapY;
	ImageWrap imageWrapZ;
	CompareOperator compareOperator;
	bool useCompare;
	Vec2F mipmapLodRange;
	float mipmapLodBias;
	VkSampler handle;
} VkSampler_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlSampler_T
{
	Window window;
	ImageFilter minImageFilter;
	ImageFilter magImageFilter;
	ImageFilter minMipmapFilter;
	bool useMipmapping;
	ImageWrap imageWrapX;
	ImageWrap imageWrapY;
	ImageWrap imageWrapZ;
	CompareOperator compareOperator;
	bool useCompare;
	Vec2F mipmapLodRange;
	float mipmapLodBias;
	GLuint handle;
} GlSampler_T;
#endif
union Sampler_T
{
	BaseSampler_T base;
#if MPGX_SUPPORT_VULKAN
	VkSampler_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlSampler_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
inline static bool getVkImageFilter(
	ImageFilter imageFilter,
	VkFilter* vkImageFilter)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		*vkImageFilter = VK_FILTER_NEAREST;
		return true;
	}
	else if (imageFilter == LINEAR_IMAGE_FILTER)
	{
		*vkImageFilter = VK_FILTER_LINEAR;
		return true;
	}
	else
	{
		return false;
	}
}
inline static bool getVkMipmapFilter(
	ImageFilter mipmapFilter,
	VkSamplerMipmapMode* vkMipmapFilter)
{
	if (mipmapFilter == NEAREST_IMAGE_FILTER)
	{
		*vkMipmapFilter = VK_SAMPLER_MIPMAP_MODE_NEAREST;
		return true;
	}
	else if (mipmapFilter == LINEAR_IMAGE_FILTER)
	{
		*vkMipmapFilter = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		return true;
	}
	else
	{
		return false;
	}
}
inline static bool getVkImageWrap(
	ImageWrap imageWrap,
	VkSamplerAddressMode* vkImageWrap)
{
	if (imageWrap == REPEAT_IMAGE_WRAP)
	{
		*vkImageWrap = VK_SAMPLER_ADDRESS_MODE_REPEAT;
		return true;
	}
	else if (imageWrap == MIRRORED_REPEAT_IMAGE_WRAP)
	{
		*vkImageWrap = VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT;
		return true;
	}
	else if (imageWrap == CLAMP_TO_EDGE_IMAGE_WRAP)
	{
		*vkImageWrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE;
		return true;
	}
	else if (imageWrap == CLAMP_TO_BORDER_IMAGE_WRAP)
	{
		*vkImageWrap = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_BORDER;
		return true;
	}
	else if (imageWrap == MIRROR_CLAMP_TO_EDGE_IMAGE_WRAP)
	{
		*vkImageWrap = VK_SAMPLER_ADDRESS_MODE_MIRROR_CLAMP_TO_EDGE;
		return true;
	}
	else
	{
		return false;
	}
}

inline static void destroyVkSampler(
	VkDevice device,
	Sampler sampler)
{
	if (sampler == NULL)
		return;

	vkDestroySampler(
		device,
		sampler->vk.handle,
		NULL);
	free(sampler);
}
inline static MpgxResult createVkSampler(
	VkDevice device,
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator compareOperator,
	bool useCompare,
	Vec2F mipmapLodRange,
	float mipmapLodBias,
	Sampler* sampler)
{
	Sampler samplerInstance = calloc(1,
		sizeof(Sampler_T));

	if (samplerInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	samplerInstance->vk.window = window;
	samplerInstance->vk.minImageFilter = minImageFilter;
	samplerInstance->vk.magImageFilter = magImageFilter;
	samplerInstance->vk.minMipmapFilter = minMipmapFilter;
	samplerInstance->vk.useMipmapping = useMipmapping;
	samplerInstance->vk.imageWrapX = imageWrapX;
	samplerInstance->vk.imageWrapY = imageWrapY;
	samplerInstance->vk.imageWrapZ = imageWrapZ;
	samplerInstance->vk.compareOperator = compareOperator;
	samplerInstance->vk.useCompare = useCompare;
	samplerInstance->vk.mipmapLodRange = mipmapLodRange;
	samplerInstance->vk.mipmapLodBias = mipmapLodBias;

	VkFilter minFilter, magFilter;
	VkSamplerMipmapMode mipmapMode;
	VkSamplerAddressMode wrapX, wrapY, wrapZ;
	VkCompareOp compare;

	bool result = getVkImageFilter(
		minImageFilter,
		&minFilter);
	result &= getVkImageFilter(
		magImageFilter,
		&magFilter);
	result &= getVkMipmapFilter(
		minMipmapFilter,
		&mipmapMode);
	result &= getVkImageWrap(
		imageWrapX,
		&wrapX);
	result &= getVkImageWrap(
		imageWrapY,
		&wrapY);
	result &= getVkImageWrap(
		imageWrapZ,
		&wrapZ);
	result &= getVkCompareOperator(
		compareOperator,
		&compare);

	if (result == false)
	{
		destroyVkSampler(
			device,
			samplerInstance);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	VkSamplerCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
		NULL,
		0,
		minFilter,
		magFilter,
		mipmapMode,
		wrapX,
		wrapY,
		wrapZ,
		mipmapLodBias,
		VK_FALSE, // TODO
		1.0f,
		useCompare ? VK_TRUE : VK_FALSE,
		compare,
		mipmapLodRange.x,
		mipmapLodRange.y,
		VK_BORDER_COLOR_FLOAT_TRANSPARENT_BLACK, // TODO:
		VK_FALSE,
	};

	VkSampler handle;

	VkResult vkResult = vkCreateSampler(
		device,
		&createInfo,
		NULL,
		&handle);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkSampler(
			device,
			samplerInstance);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	samplerInstance->vk.handle = handle;

	*sampler = samplerInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static bool getGlImageFilter(
	ImageFilter imageFilter,
	ImageFilter mipmapFilter,
	bool useMipmapping,
	GLenum* glImageFilter)
{
	if (imageFilter == NEAREST_IMAGE_FILTER)
	{
		if (useMipmapping == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
			{
				*glImageFilter = GL_NEAREST_MIPMAP_NEAREST;
				return true;
			}
			else if (mipmapFilter == LINEAR_IMAGE_FILTER)
			{
				*glImageFilter = GL_NEAREST_MIPMAP_LINEAR;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			*glImageFilter = GL_NEAREST;
			return true;
		}
	}
	else if (imageFilter == LINEAR_IMAGE_FILTER)
	{
		if (useMipmapping == true)
		{
			if (mipmapFilter == NEAREST_IMAGE_FILTER)
			{
				*glImageFilter = GL_LINEAR_MIPMAP_NEAREST;
				return true;
			}
			else if (mipmapFilter == LINEAR_IMAGE_FILTER)
			{
				*glImageFilter = GL_LINEAR_MIPMAP_LINEAR;
				return true;
			}
			else
			{
				return false;
			}
		}
		else
		{
			*glImageFilter = GL_LINEAR;
			return true;
		}
	}
	else
	{
		return false;
	}
}
inline static bool getGlImageWrap(
	ImageWrap imageWrap,
	GLenum* glImageWrap)
{
	if (imageWrap == REPEAT_IMAGE_WRAP)
	{
		*glImageWrap = GL_REPEAT;
		return true;
	}
	else if (imageWrap == MIRRORED_REPEAT_IMAGE_WRAP)
	{
		*glImageWrap = GL_MIRRORED_REPEAT;
		return true;
	}
	else if (imageWrap == CLAMP_TO_EDGE_IMAGE_WRAP)
	{
		*glImageWrap = GL_CLAMP_TO_EDGE;
		return true;
	}
	else
	{
		return false;
	}
}

inline static void destroyGlSampler(
	Sampler sampler)
{
	if (sampler == NULL)
		return;

	makeWindowContextCurrent(
		sampler->gl.window);

	glDeleteSamplers(
		GL_ONE,
		&sampler->gl.handle);
	assertOpenGL();

	free(sampler);
}
inline static MpgxResult createGlSampler(
	Window window,
	ImageFilter minImageFilter,
	ImageFilter magImageFilter,
	ImageFilter minMipmapFilter,
	bool useMipmapping,
	ImageWrap imageWrapX,
	ImageWrap imageWrapY,
	ImageWrap imageWrapZ,
	CompareOperator compareOperator,
	bool useCompare,
	Vec2F mipmapLodRange,
	Sampler* sampler)
{
	Sampler samplerInstance = calloc(1,
		sizeof(Sampler_T));

	if (samplerInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	samplerInstance->gl.window = window;
	samplerInstance->gl.minImageFilter = minImageFilter;
	samplerInstance->gl.magImageFilter = magImageFilter;
	samplerInstance->gl.minMipmapFilter = minMipmapFilter;
	samplerInstance->gl.useMipmapping = useMipmapping;
	samplerInstance->gl.imageWrapX = imageWrapX;
	samplerInstance->gl.imageWrapY = imageWrapY;
	samplerInstance->gl.imageWrapZ = imageWrapZ;
	samplerInstance->gl.compareOperator = compareOperator;
	samplerInstance->gl.useCompare = useCompare;
	samplerInstance->gl.mipmapLodRange = mipmapLodRange;
	samplerInstance->gl.mipmapLodBias = 0.0f;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenSamplers(
		GL_ONE,
		&handle);

	samplerInstance->gl.handle = handle;

	GLenum minFilter, magFilter,
		wrapX, wrapY, wrapZ, compare;

	bool result = getGlImageFilter(
		minImageFilter,
		minMipmapFilter,
		useMipmapping,
		&minFilter);
	result &= getGlImageFilter(
		magImageFilter,
		magImageFilter,
		false,
		&magFilter);
	result &= getGlImageWrap(
		imageWrapX,
		&wrapX);
	result &= getGlImageWrap(
		imageWrapY,
		&wrapY);
	result &= getGlImageWrap(
		imageWrapZ,
		&wrapZ);
	result &= getGlCompareOperator(
		compareOperator,
		&compare);

	if (result == false)
	{
		destroyGlSampler(samplerInstance);
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	GLint compareMode = useCompare ?
		GL_COMPARE_REF_TO_TEXTURE : GL_NONE;

	glSamplerParameteri(
		handle,
		GL_TEXTURE_MIN_FILTER,
		(GLint)minFilter);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_MAG_FILTER,
		(GLint)magFilter);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_S,
		(GLint)wrapX);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_T,
		(GLint)wrapY);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_R,
		(GLint)wrapZ);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_MODE,
		compareMode);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_FUNC,
		(GLint)compare);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MIN_LOD,
		(GLfloat)mipmapLodRange.x);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MAX_LOD,
		(GLfloat)mipmapLodRange.y);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlSampler(samplerInstance);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*sampler = samplerInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
