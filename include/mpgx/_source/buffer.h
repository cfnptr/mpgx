#pragma once

typedef struct _VkBuffer
{
	Window window;
	uint8_t type;
	size_t size;
	bool isConstant;
#if MPGX_SUPPORT_VULKAN
	VkBuffer handle;
#endif
} _VkBuffer;
typedef struct _GlBuffer
{
	Window window;
	uint8_t type;
	size_t size;
	bool isConstant;
	GLenum glType;
	GLuint handle;
} _GlBuffer;
union Buffer
{
	_VkBuffer vk;
	_GlBuffer gl;
};

inline static Buffer createVkBuffer(
	Window window,
	uint8_t type,
	const void* data,
	size_t size,
	bool isConstant)
{
#if MPGX_VULKAN_SUPPORT
	// TODO:
	abort();
#else
	abort();
#endif
}
inline static Buffer createGlBuffer(
	Window window,
	uint8_t type,
	const void* data,
	size_t size,
	bool isConstant)
{
	Buffer buffer = malloc(
		sizeof(union Buffer));

	if (buffer == NULL)
		return NULL;

	GLenum glType;

	if (type == VERTEX_BUFFER_TYPE)
	{
		glType = GL_ARRAY_BUFFER;
	}
	else if (type == INDEX_BUFFER_TYPE)
	{
		glType = GL_ELEMENT_ARRAY_BUFFER;
	}
	else if (type == UNIFORM_BUFFER_TYPE)
	{
		glType = GL_UNIFORM_BUFFER;
	}
	else
	{
		free(buffer);
		return NULL;
	}

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenBuffers(
		GL_ONE,
		&handle);

	GLenum usage = isConstant ?
		GL_STATIC_DRAW :
		GL_DYNAMIC_DRAW;

	glBindBuffer(
		glType,
		handle);
	glBufferData(
		glType,
		(GLsizeiptr)(size),
		data,
		usage);

	assertOpenGL();

	buffer->gl.window = window;
	buffer->gl.type = type;
	buffer->gl.size = size;
	buffer->gl.isConstant = isConstant;
	buffer->gl.glType = glType;
	buffer->gl.handle = handle;
	return buffer;
}

inline static void destroyVkBuffer(Buffer buffer)
{
	// TODO:
}
inline static void destroyGlBuffer(Buffer buffer)
{
	makeWindowContextCurrent(
		buffer->gl.window);

	glDeleteBuffers(
		GL_ONE,
		&buffer->gl.handle);
	assertOpenGL();

	free(buffer);
}

inline static void setVkBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	// TODO:
}
inline static void setGlBufferData(
	Buffer buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	glBindBuffer(
		buffer->gl.glType,
		buffer->gl.handle);
	glBufferSubData(
		buffer->gl.glType,
		(GLintptr)offset,
		(GLsizeiptr)size,
		data);
	assertOpenGL();
}
