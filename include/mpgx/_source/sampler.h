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
	int handle;
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
	GLuint handle;
} _GlSampler;
union Sampler
{
	_VkSampler vk;
	_GlSampler gl;
};

inline static Sampler createVkSampler(
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
	// TODO:
	abort();
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

	glSamplerParameteri(
		handle,
		GL_TEXTURE_MIN_FILTER,
		(GLint)getGlImageFilter(
			minImageFilter,
			minMipmapFilter,
			useMipmapping));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_MAG_FILTER,
		(GLint)getGlImageFilter(
			magImageFilter,
			magImageFilter,
			false));

	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_S,
		(GLint)getGlImageWrap(imageWrapX));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_T,
		(GLint)getGlImageWrap(imageWrapY));
	glSamplerParameteri(
		handle,
		GL_TEXTURE_WRAP_R,
		(GLint)getGlImageWrap(imageWrapZ));

	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_MODE,
		useCompare ?
		GL_COMPARE_REF_TO_TEXTURE :
		GL_NONE);
	glSamplerParameteri(
		handle,
		GL_TEXTURE_COMPARE_FUNC,
		(GLint)getGlImageCompare(imageCompare));

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
	sampler->gl.handle = handle;
	return sampler;
}

inline static void destroyVkSampler(Sampler sampler)
{
	// TODO:
}
inline static void destroyGlSampler(Sampler sampler)
{
	makeWindowContextCurrent(
		sampler->gl.window);

	glDeleteSamplers(
		GL_ONE,
		&sampler->gl.handle);
	assertOpenGL();

	free(sampler);
}
