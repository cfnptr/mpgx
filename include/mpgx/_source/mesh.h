#pragma once
#include "mpgx/_source/buffer.h"
#include "mpgx/_source/pipeline.h"

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

#if MPGX_SUPPORT_VULKAN
inline static Mesh createVkMesh(
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

	mesh->gl.window = window;
	mesh->gl.drawIndex = drawIndex;
	mesh->gl.indexCount = indexCount;
	mesh->gl.indexOffset = indexOffset;
	mesh->gl.vertexBuffer = vertexBuffer;
	mesh->gl.indexBuffer = indexBuffer;
	return mesh;
}
inline static void destroyVkMesh(
	Mesh mesh)
{
	free(mesh);
}
inline static void drawVkMesh(
	VkCommandBuffer commandBuffer,
	Mesh mesh)
{
	uint8_t drawIndex = mesh->vk.drawIndex;

	VkIndexType vkDrawIndex;
	VkDeviceSize vkIndexOffset;

	if (drawIndex == UINT16_DRAW_INDEX)
	{
		vkDrawIndex = VK_INDEX_TYPE_UINT16;
		vkIndexOffset = mesh->vk.indexOffset * sizeof(uint16_t);
	}
	else if (drawIndex == UINT32_DRAW_INDEX)
	{
		vkDrawIndex = VK_INDEX_TYPE_UINT32;
		vkIndexOffset = mesh->vk.indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	VkBuffer buffer = mesh->vk.vertexBuffer->vk.handle;
	const VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(
		commandBuffer,
		0,
		1,
		&buffer,
		&offset);
	vkCmdBindIndexBuffer(
		commandBuffer,
		mesh->vk.indexBuffer->vk.handle,
		vkIndexOffset,
		vkDrawIndex);
	vkCmdDrawIndexed(
		commandBuffer,
		mesh->vk.indexCount,
		1,
		0,
		0,
		0);
}
#endif

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
inline static void destroyGlMesh(
	Mesh mesh)
{
	makeWindowContextCurrent(
		mesh->gl.window);

	glDeleteVertexArrays(
		GL_ONE,
		&mesh->gl.handle);
	assertOpenGL();

	free(mesh);
}
inline static void drawGlMesh(
	Mesh mesh,
	Pipeline pipeline)
{
	glBindVertexArray(
		mesh->gl.handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		mesh->gl.vertexBuffer->gl.handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		mesh->gl.indexBuffer->gl.handle);
	assertOpenGL();

	if (pipeline->gl.onUniformsSet != NULL)
		pipeline->gl.onUniformsSet(pipeline);

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
		pipeline->gl.drawMode,
		(GLsizei)mesh->gl.indexCount,
		glDrawIndex,
		(const void*)glIndexOffset);

	assertOpenGL();
}
