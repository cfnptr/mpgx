#pragma once

typedef struct _VkImage
{
	Window window;
	uint8_t type;
	uint8_t format;
	Vec3U size;
	int handle;
} _VkImage;
typedef struct _GlImage
{
	Window window;
	uint8_t type;
	uint8_t format;
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

inline static Image createVkImage(
	Window window,
	uint8_t type,
	uint8_t format,
	Vec3U size,
	const void** data,
	uint8_t levelCount)
{
	// TODO:
}
inline static Image createGlImage(
	Window window,
	uint8_t type,
	uint8_t format,
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
		glType = GL_TEXTURE_2D;
	else if (type == IMAGE_3D_TYPE)
		glType = GL_TEXTURE_3D;
	else
		abort();

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
				GL_TEXTURE_2D,
				GL_TEXTURE_BASE_LEVEL,
				0);
			glTexParameteri(
				GL_TEXTURE_2D,
				GL_TEXTURE_MAX_LEVEL,
				levelCount - 1);
		}
	}

	assertOpenGL();

	image->gl.window = window;
	image->gl.type = type;
	image->gl.format = format;
	image->gl.size = size;
	image->gl.handle = handle;
	image->gl.glType = glType;
	image->gl.dataType = dataType;
	image->gl.dataFormat = dataFormat;
	image->gl.handle = handle;
	return image;
}

inline static void destroyVkImage(Image image)
{
	// TODO:
}
inline static void destroyGlImage(Image image)
{
	makeWindowContextCurrent(
		image->gl.window);

	glDeleteTextures(
		GL_ONE,
		&image->gl.handle);
	assertOpenGL();

	free(image);
}

inline static void setVkImageData(
	Image image,
	const void* data,
	Vec3U size,
	Vec3U offset)
{
	// TODO:
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

	uint8_t type = image->gl.type;

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
