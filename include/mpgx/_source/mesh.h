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
#include "mpgx/_source/buffer.h"
#include "mpgx/_source/pipeline.h"

typedef struct BaseMesh_T
{
	Window window;
	IndexType indexType;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
} BaseMesh_T;
typedef struct VkMesh_T
{
	Window window;
	IndexType indexType;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
} VkMesh_T;
typedef struct GlMesh_T
{
	Window window;
	IndexType indexType;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	GLuint handle;
} GlMesh_T;
union Mesh_T
{
	BaseMesh_T base;
	VkMesh_T vk;
	GlMesh_T gl;
};

#if MPGX_SUPPORT_VULKAN
inline static Mesh createVkMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	Mesh mesh = calloc(1, sizeof(Mesh_T));

	if (mesh == NULL)
		return NULL;

	mesh->gl.window = window;
	mesh->gl.indexType = indexType;
	mesh->gl.indexCount = indexCount;
	mesh->gl.indexOffset = indexOffset;
	mesh->gl.vertexBuffer = vertexBuffer;
	mesh->gl.indexBuffer = indexBuffer;
	return mesh;
}
inline static void destroyVkMesh(
	Mesh mesh,
	bool destroyBuffers)
{
	if (mesh == NULL)
		return;

	if (destroyBuffers == true)
	{
		destroyBuffer(mesh->vk.vertexBuffer);
		destroyBuffer(mesh->vk.indexBuffer);
	}

	free(mesh);
}
inline static void drawVkMesh(
	VkCommandBuffer commandBuffer,
	Mesh mesh)
{
	IndexType indexType = mesh->vk.indexType;

	VkIndexType vkDrawIndex;
	VkDeviceSize vkIndexOffset;

	if (indexType == UINT16_INDEX_TYPE)
	{
		vkDrawIndex = VK_INDEX_TYPE_UINT16;
		vkIndexOffset = mesh->vk.indexOffset * sizeof(uint16_t);
	}
	else if (indexType == UINT32_INDEX_TYPE)
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
		(uint32_t)mesh->vk.indexCount,
		1,
		0,
		0,
		0);
}
#endif

inline static void destroyGlMesh(
	Mesh mesh,
	bool destroyBuffers)
{
	if (mesh == NULL)
		return;

	makeWindowContextCurrent(
		mesh->gl.window);

	glDeleteVertexArrays(
		GL_ONE,
		&mesh->gl.handle);
	assertOpenGL();

	if (destroyBuffers == true)
	{
		destroyBuffer(mesh->gl.vertexBuffer);
		destroyBuffer(mesh->gl.indexBuffer);
	}

	free(mesh);
}
inline static Mesh createGlMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	Mesh mesh = calloc(1, sizeof(Mesh_T));

	if (mesh == NULL)
		return NULL;

	mesh->gl.window = window;
	mesh->gl.indexType = indexType;
	mesh->gl.indexCount = indexCount;
	mesh->gl.indexOffset = indexOffset;
	mesh->gl.vertexBuffer = vertexBuffer;
	mesh->gl.indexBuffer = indexBuffer;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);

	mesh->gl.handle = handle;

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlMesh(
			mesh,
			false);
		return NULL;
	}

	return mesh;
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

	IndexType drawIndex = mesh->gl.indexType;

	GLenum glDrawIndex;
	size_t glIndexOffset;

	if (drawIndex == UINT16_INDEX_TYPE)
	{
		glDrawIndex = GL_UNSIGNED_SHORT;
		glIndexOffset = mesh->gl.indexOffset * sizeof(uint16_t);
	}
	else if (drawIndex == UINT32_INDEX_TYPE)
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
