#pragma once
#include "mpgx/_source/opengl.h"

#if MPGX_SUPPORT_VULKAN
#include "mpgx/_source/vulkan.h"
#endif

typedef struct _VkSampler
{
	Window window;
	uint8_t minImageFilter;
	uint8_t magImageFilter;
	uint8_t minMipmapFilter;
	bool useMipmapping;
	uint8_t imageWrapX;
	uint8_t imageWrapY;
	uint8_t imageWrapZ;
	uint8_t compareOperation;
	bool useCompare;
	float minMipmapLod;
	float maxMipmapLod;
	float mipmapLodBias;
#if MPGX_SUPPORT_VULKAN
	VkSampler handle;
#endif
} _VkSampler;
typedef struct _GlSampler
{
	Window window;
	uint8_t minImageFilter;
	uint8_t magImageFilter;
	uint8_t minMipmapFilter;
	bool useMipmapping;
	uint8_t imageWrapX;
	uint8_t imageWrapY;
	uint8_t imageWrapZ;
	uint8_t compareOperation;
	bool useCompare;
	float minMipmapLod;
	float maxMipmapLod;
	float mipmapLodBias;
	GLuint handle;
} _GlSampler;
union Sampler
{
	_VkSampler vk;
	_GlSampler gl;
};

#if MPGX_SUPPORT_VULKAN
inline static bool getVkImageFilter(
	uint8_t imageFilter,
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
	uint8_t mipmapFilter,
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
	uint8_t imageWrap,
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
	uint8_t minImageFilter,
	uint8_t magImageFilter,
	uint8_t minMipmapFilter,
	bool useMipmapping,
	uint8_t imageWrapX,
	uint8_t imageWrapY,
	uint8_t imageWrapZ,
	uint8_t compareOperation,
	bool useCompare,
	float minMipmapLod,
	float maxMipmapLod,
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
	result &= getVkCompareOperation(
		compareOperation,
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
		minMipmapLod,
		maxMipmapLod,
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

	sampler->gl.window = window;
	sampler->gl.minImageFilter = minImageFilter;
	sampler->gl.magImageFilter = magImageFilter;
	sampler->gl.minMipmapFilter = minMipmapFilter;
	sampler->gl.useMipmapping = useMipmapping;
	sampler->gl.imageWrapX = imageWrapX;
	sampler->gl.imageWrapY = imageWrapY;
	sampler->gl.imageWrapZ = imageWrapZ;
	sampler->gl.compareOperation = compareOperation;
	sampler->gl.useCompare = useCompare;
	sampler->gl.minMipmapLod = minMipmapLod;
	sampler->gl.maxMipmapLod = maxMipmapLod;
	sampler->gl.mipmapLodBias = mipmapLodBias;
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
	uint8_t imageFilter,
	uint8_t mipmapFilter,
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
	uint8_t imageWrap,
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
	uint8_t minImageFilter,
	uint8_t magImageFilter,
	uint8_t minMipmapFilter,
	bool useMipmapping,
	uint8_t imageWrapX,
	uint8_t imageWrapY,
	uint8_t imageWrapZ,
	uint8_t compareOperation,
	bool useCompare,
	float minMipmapLod,
	float maxMipmapLod)
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
		wrapX, wrapY, wrapZ,
		compareOperator;

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
	result &= getGlCompareOperation(
		compareOperation,
		&compareOperator);

	if (result == false)
	{
		glDeleteSamplers(
			GL_ONE,
			&handle);
		free(sampler);
		return NULL;
	}

	GLint glCompareMode = useCompare ?
		GL_COMPARE_REF_TO_TEXTURE : GL_NONE;

	glSamplerParameteri(
		handle,
		GL_TEXTURE_MIN_FILTER,
		(GLint)minImageFilter);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_MAG_FILTER,
		(GLint)magImageFilter);
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
		glCompareMode);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_FUNC,
		(GLint)compareOperator);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MIN_LOD,
		(GLfloat)minMipmapLod);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MAX_LOD,
		(GLfloat)maxMipmapLod);

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
	sampler->gl.compareOperation = compareOperation;
	sampler->gl.useCompare = useCompare;
	sampler->gl.minMipmapLod = minMipmapLod;
	sampler->gl.maxMipmapLod = maxMipmapLod;
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
