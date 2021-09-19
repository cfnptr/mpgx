#pragma once

// TODO: possibly set default precision
// or detect supported precisions

#define OPENGL_SHADER_HEADER \
	"#version 330 core\n"
#define OPENGL_ES_SHADER_HEADER \
	"#version 300 es\n"

typedef struct _VkShader
{
	Window window;
	uint8_t type;
#if MPGX_SUPPORT_VULKAN
	VkShaderModule handle;
#endif
} _VkShader;
typedef struct _GlShader
{
	Window window;
	uint8_t type;
	GLuint handle;
} _GlShader;
union Shader
{
	_GlShader gl;
	_VkShader vk;
};

#if MPGX_SUPPORT_VULKAN
inline static Shader createVkShader(
	VkDevice device,
	Window window,
	uint8_t type,
	const void* code,
	size_t size)
{
	Shader shader = malloc(
		sizeof(union Shader));

	if (shader == NULL)
		return NULL;

	VkShaderModuleCreateInfo shaderModuleCreateInfo = {
		VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
		NULL,
		0,
		size,
		code,
	};

	VkShaderModule handle;

	VkResult result = vkCreateShaderModule(
		device,
		&shaderModuleCreateInfo,
		NULL,
		&handle);

	if (result != VK_SUCCESS)
	{
		free(shader);
		return NULL;
	}

	shader->vk.window = window;
	shader->vk.type = type;
	shader->vk.handle = handle;
	return shader;
}
#endif

inline static Shader createGlShader(
	Window window,
	uint8_t type,
	const void* code,
	uint8_t api)
{
	Shader shader = malloc(
		sizeof(union Shader));

	if (shader == NULL)
		return NULL;

	GLenum glType;

	if (type == VERTEX_SHADER_TYPE)
	{
		glType = GL_VERTEX_SHADER;
	}
	else if (type == FRAGMENT_SHADER_TYPE)
	{
		glType = GL_FRAGMENT_SHADER;
	}
	else if (type == COMPUTE_SHADER_TYPE)
	{
		glType = GL_COMPUTE_SHADER;
	}
	else
	{
		free(shader);
		return NULL;
	}

	const char* sources[2];

	if (api == OPENGL_GRAPHICS_API)
		sources[0] = OPENGL_SHADER_HEADER;
	else if (api == OPENGL_ES_GRAPHICS_API)
		sources[0] = OPENGL_ES_SHADER_HEADER;
	else
		abort();

	sources[1] = (const char*)code;

	makeWindowContextCurrent(window);

	GLuint handle = glCreateShader(glType);

	glShaderSource(
		handle,
		2,
		sources,
		NULL);

	glCompileShader(handle);

	GLint result;

	glGetShaderiv(
		handle,
		GL_COMPILE_STATUS,
		&result);

	if (result == GL_FALSE)
	{
		GLint length = 0;

		glGetShaderiv(
			handle,
			GL_INFO_LOG_LENGTH,
			&length);

		if (length > 0)
		{
			char* infoLog = malloc(
				length * sizeof(char));

			if (infoLog == NULL)
			{
				glDeleteShader(handle);
				free(shader);
				return NULL;
			}

			glGetShaderInfoLog(
				handle,
				length,
				&length,
				(GLchar*)infoLog);

			const char* typeString;

			if (type == VERTEX_SHADER_TYPE)
				typeString = "vertex";
			else if (type == FRAGMENT_SHADER_TYPE)
				typeString = "fragment";
			else if (type == COMPUTE_SHADER_TYPE)
				typeString = "compute";
			else
				abort();

			fprintf(GL_INFO_LOG_OUT,
				"OpenGL %s shader compile error: %s\n",
				typeString,
				infoLog);
			free(infoLog);
		}

		assertOpenGL();

		glDeleteShader(handle);
		free(shader);
		return NULL;
	}

	assertOpenGL();

	shader->gl.window = window;
	shader->gl.type = type;
	shader->gl.handle = handle;
	return shader;
}

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkShader(
	VkDevice device,
	Shader shader)
{
	vkDestroyShaderModule(
		device,
		shader->vk.handle,
		NULL);
	free(shader);
}
#endif

inline static void destroyGlShader(
	Shader shader)
{
	makeWindowContextCurrent(
		shader->gl.window);

	glDeleteShader(shader->gl.handle);
	assertOpenGL();

	free(shader);
}
