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

typedef struct _BaseSampler
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
} _BaseSampler;
typedef struct _VkSampler
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
#if MPGX_SUPPORT_VULKAN
	VkSampler handle;
#endif
} _VkSampler;
typedef struct _GlSampler
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
} _GlSampler;
union Sampler
{
	_BaseSampler base;
	_VkSampler vk;
	_GlSampler gl;
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
inline static Sampler createVkSampler(
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
	float mipmapLodBias)
{
	Sampler sampler = malloc(
		sizeof(union Sampler));

	if (sampler == NULL)
		return NULL;

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
		free(sampler);
		return NULL;
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
		0.0f,
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
		free(sampler);
		return NULL;
	}

	sampler->vk.window = window;
	sampler->vk.minImageFilter = minImageFilter;
	sampler->vk.magImageFilter = magImageFilter;
	sampler->vk.minMipmapFilter = minMipmapFilter;
	sampler->vk.useMipmapping = useMipmapping;
	sampler->vk.imageWrapX = imageWrapX;
	sampler->vk.imageWrapY = imageWrapY;
	sampler->vk.imageWrapZ = imageWrapZ;
	sampler->vk.compareOperator = compareOperator;
	sampler->vk.useCompare = useCompare;
	sampler->vk.mipmapLodRange = mipmapLodRange;
	sampler->vk.mipmapLodBias = mipmapLodBias;
	sampler->vk.handle = handle;
	return sampler;
}
inline static void destroyVkSampler(
	VkDevice device,
	Sampler sampler)
{
	vkDestroySampler(
		device,
		sampler->vk.handle,
		NULL);
	free(sampler);
}
#endif

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
inline static Sampler createGlSampler(
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
	Vec2F mipmapLodRange)
{
	Sampler sampler = malloc(
		sizeof(union Sampler));

	if (sampler == NULL)
		return NULL;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenSamplers(
		GL_ONE,
		&handle);

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
		glDeleteSamplers(
			GL_ONE,
			&handle);
		free(sampler);
		return NULL;
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
		glDeleteSamplers(
			GL_ONE,
			&handle);
		free(sampler);
		return NULL;
	}

	sampler->gl.window = window;
	sampler->gl.minImageFilter = minImageFilter;
	sampler->gl.magImageFilter = magImageFilter;
	sampler->gl.minMipmapFilter = minMipmapFilter;
	sampler->gl.useMipmapping = useMipmapping;
	sampler->gl.imageWrapX = imageWrapX;
	sampler->gl.imageWrapY = imageWrapY;
	sampler->gl.imageWrapZ = imageWrapZ;
	sampler->gl.compareOperator = compareOperator;
	sampler->gl.useCompare = useCompare;
	sampler->gl.mipmapLodRange = mipmapLodRange;
	sampler->gl.mipmapLodBias = 0.0f;
	sampler->gl.handle = handle;
	return sampler;
}
inline static void destroyGlSampler(
	Sampler sampler)
{
	makeWindowContextCurrent(
		sampler->gl.window);

	glDeleteSamplers(
		GL_ONE,
		&sampler->gl.handle);
	assertOpenGL();

	free(sampler);
}
