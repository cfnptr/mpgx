#pragma once

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
	uint8_t imageCompare;
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
	uint8_t imageCompare;
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
	uint8_t imageCompare,
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
	result &= getVkImageCompare(
		imageCompare,
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
	sampler->gl.imageCompare = imageCompare;
	sampler->gl.useCompare = useCompare;
	sampler->gl.minMipmapLod = minMipmapLod;
	sampler->gl.maxMipmapLod = maxMipmapLod;
	sampler->gl.mipmapLodBias = mipmapLodBias;
	sampler->vk.handle = handle;
	return sampler;
}
#endif

inline static Sampler createGlSampler(
	Window window,
	uint8_t minImageFilter,
	uint8_t magImageFilter,
	uint8_t minMipmapFilter,
	bool useMipmapping,
	uint8_t imageWrapX,
	uint8_t imageWrapY,
	uint8_t imageWrapZ,
	uint8_t imageCompare,
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
	result &= getGlImageCompare(
		imageCompare,
		&compare);

	if (result == false)
	{
		glDeleteSamplers(
			GL_ONE,
			&handle);
		free(sampler);
		return NULL;
	}

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
		useCompare ?
		GL_COMPARE_REF_TO_TEXTURE :
		GL_NONE);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_FUNC,
		(GLint)compare);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MIN_LOD,
		(GLfloat)minMipmapLod);
	glSamplerParameterf(
		handle,
		GL_TEXTURE_MAX_LOD,
		(GLfloat)maxMipmapLod);

	assertOpenGL();

	sampler->gl.window = window;
	sampler->gl.minImageFilter = minImageFilter;
	sampler->gl.magImageFilter = magImageFilter;
	sampler->gl.minMipmapFilter = minMipmapFilter;
	sampler->gl.useMipmapping = useMipmapping;
	sampler->gl.imageWrapX = imageWrapX;
	sampler->gl.imageWrapY = imageWrapY;
	sampler->gl.imageWrapZ = imageWrapZ;
	sampler->gl.imageCompare = imageCompare;
	sampler->gl.useCompare = useCompare;
	sampler->gl.minMipmapLod = minMipmapLod;
	sampler->gl.maxMipmapLod = maxMipmapLod;
	sampler->gl.mipmapLodBias = 0.0f;
	sampler->gl.handle = handle;
	return sampler;
}

#if MPGX_SUPPORT_VULKAN
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
