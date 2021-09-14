#pragma once

typedef struct _VkMesh
{
	Window window;
	uint8_t drawIndex;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
} _VkMesh;
typedef struct _GlMesh
{
	Window window;
	uint8_t drawIndex;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	GLuint handle;
} _GlMesh;
union Mesh
{
	_VkMesh vk;
	_GlMesh gl;
};

inline static Mesh createVkMesh(
	Window window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	// TODO:
}
inline static Mesh createGlMesh(
	Window window,
	uint8_t drawIndex,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	Mesh mesh = malloc(
		sizeof(union Mesh));

	if (mesh == NULL)
		return NULL;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);
	assertOpenGL();

	mesh->gl.window = window;
	mesh->gl.drawIndex = drawIndex;
	mesh->gl.indexCount = indexCount;
	mesh->gl.indexOffset = indexOffset;
	mesh->gl.vertexBuffer = vertexBuffer;
	mesh->gl.indexBuffer = indexBuffer;
	mesh->gl.handle = handle;
	return mesh;
}

inline static void destroyVkMesh(Mesh mesh)
{
	// TODO:
}
inline static void destroyGlMesh(Mesh mesh)
{
	makeWindowContextCurrent(
		mesh->gl.window);

	glDeleteVertexArrays(
		GL_ONE,
		&mesh->gl.handle);
	assertOpenGL();

	free(mesh);
}

inline static void drawVkMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	// TODO:
}
inline static void drawGlMesh(
	Mesh mesh,
	Pipeline pipeline,
	OnPipelineUniformsSet onUniformsSet,
	uint8_t drawMode)
{
	Buffer vertexBuffer = mesh->gl.vertexBuffer;
	Buffer indexBuffer = mesh->gl.indexBuffer;

	glBindVertexArray(
		mesh->gl.handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		vertexBuffer->gl.handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		indexBuffer->gl.handle);
	assertOpenGL();

	onUniformsSet(pipeline);

	GLenum glDrawMode;

	switch (drawMode)
	{
	default:
		abort();
	case POINTS_DRAW_MODE:
		glDrawMode = GL_POINTS;
		break;
	case LINE_STRIP_DRAW_MODE:
		glDrawMode = GL_LINE_STRIP;
		break;
	case LINE_LOOP_DRAW_MODE:
		glDrawMode = GL_LINE_LOOP;
		break;
	case LINES_DRAW_MODE:
		glDrawMode = GL_LINES;
		break;
	case TRIANGLE_STRIP_DRAW_MODE:
		glDrawMode = GL_TRIANGLE_STRIP;
		break;
	case TRIANGLE_FAN_DRAW_MODE:
		glDrawMode = GL_TRIANGLE_FAN;
		break;
	case TRIANGLES_DRAW_MODE:
		glDrawMode = GL_TRIANGLES;
		break;
	}

	uint8_t drawIndex = mesh->gl.drawIndex;

	GLenum glDrawIndex;
	size_t glIndexOffset;

	if (drawIndex == UINT16_DRAW_INDEX)
	{
		glDrawIndex = GL_UNSIGNED_SHORT;
		glIndexOffset = mesh->gl.indexOffset * sizeof(uint16_t);
	}
	else if (drawIndex == UINT32_DRAW_INDEX)
	{
		glDrawIndex = GL_UNSIGNED_INT;
		glIndexOffset = mesh->gl.indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	glDrawElements(
		glDrawMode,
		(GLsizei)mesh->gl.indexCount,
		glDrawIndex,
		(const void*)glIndexOffset);

	assertOpenGL();
}
