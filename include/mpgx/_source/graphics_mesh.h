// Copyright 2020-2022 Nikita Fediuchin. All rights reserved.
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
#include "mpgx/_source/graphics_pipeline.h"

typedef struct BaseGraphicsMesh_T
{
	Window window;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	IndexType indexType;
} BaseGraphicsMesh_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkGraphicsMesh_T
{
	Window window;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	IndexType indexType;
} VkGraphicsMesh_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlGraphicsMesh_T
{
	Window window;
	size_t indexCount;
	size_t indexOffset;
	Buffer vertexBuffer;
	Buffer indexBuffer;
	IndexType indexType;
	uint8_t _alignment[3];
	GLuint handle;
} GlGraphicsMesh_T;
#endif
union GraphicsMesh_T
{
	BaseGraphicsMesh_T base;
#if MPGX_SUPPORT_VULKAN
	VkGraphicsMesh_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlGraphicsMesh_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult createVkGraphicsMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	GraphicsMesh* graphicsMesh)
{
	assert(window);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(graphicsMesh);

	GraphicsMesh graphicsMeshInstance = calloc(1,
		sizeof(GraphicsMesh_T));

	if (!graphicsMeshInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	graphicsMeshInstance->vk.window = window;
	graphicsMeshInstance->vk.indexCount = indexCount;
	graphicsMeshInstance->vk.indexOffset = indexOffset;
	graphicsMeshInstance->vk.vertexBuffer = vertexBuffer;
	graphicsMeshInstance->vk.indexBuffer = indexBuffer;
	graphicsMeshInstance->vk.indexType = indexType;

	*graphicsMesh = graphicsMeshInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static void destroyVkGraphicsMesh(
	GraphicsMesh graphicsMesh,
	bool destroyBuffers)
{
	if (!graphicsMesh)
		return;

	if (destroyBuffers)
	{
		destroyBuffer(graphicsMesh->vk.vertexBuffer);
		destroyBuffer(graphicsMesh->vk.indexBuffer);
	}

	free(graphicsMesh);
}
inline static void drawVkGraphicsMesh(
	VkCommandBuffer commandBuffer,
	GraphicsMesh graphicsMesh)
{
	assert(commandBuffer);
	assert(graphicsMesh);

	IndexType indexType = graphicsMesh->vk.indexType;

	VkIndexType vkDrawIndex;
	VkDeviceSize vkIndexOffset;

	if (indexType == UINT16_INDEX_TYPE)
	{
		vkDrawIndex = VK_INDEX_TYPE_UINT16;
		vkIndexOffset = graphicsMesh->vk.indexOffset * sizeof(uint16_t);
	}
	else if (indexType == UINT32_INDEX_TYPE)
	{
		vkDrawIndex = VK_INDEX_TYPE_UINT32;
		vkIndexOffset = graphicsMesh->vk.indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	VkBuffer buffer = graphicsMesh->vk.vertexBuffer->vk.handle;
	const VkDeviceSize offset = 0;

	vkCmdBindVertexBuffers(
		commandBuffer,
		0,
		1,
		&buffer,
		&offset);
	vkCmdBindIndexBuffer(
		commandBuffer,
		graphicsMesh->vk.indexBuffer->vk.handle,
		vkIndexOffset,
		vkDrawIndex);
	vkCmdDrawIndexed(
		commandBuffer,
		(uint32_t)graphicsMesh->vk.indexCount,
		1,
		0,
		0,
		0);
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static void destroyGlGraphicsMesh(
	GraphicsMesh graphicsMesh,
	bool destroyBuffers)
{
	if (!graphicsMesh)
		return;

	makeWindowContextCurrent(
		graphicsMesh->gl.window);

	glDeleteVertexArrays(
		GL_ONE,
		&graphicsMesh->gl.handle);
	assertOpenGL();

	if (destroyBuffers)
	{
		destroyBuffer(graphicsMesh->gl.vertexBuffer);
		destroyBuffer(graphicsMesh->gl.indexBuffer);
	}

	free(graphicsMesh);
}
inline static MpgxResult createGlGraphicsMesh(
	Window window,
	IndexType indexType,
	size_t indexCount,
	size_t indexOffset,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	GraphicsMesh* graphicsMesh)
{
	assert(window);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(graphicsMesh);

	GraphicsMesh graphicsMeshInstance = calloc(1,
		sizeof(GraphicsMesh_T));

	if (!graphicsMesh)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	graphicsMeshInstance->gl.window = window;
	graphicsMeshInstance->gl.indexCount = indexCount;
	graphicsMeshInstance->gl.indexOffset = indexOffset;
	graphicsMeshInstance->gl.vertexBuffer = vertexBuffer;
	graphicsMeshInstance->gl.indexBuffer = indexBuffer;
	graphicsMeshInstance->gl.indexType = indexType;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenVertexArrays(
		GL_ONE,
		&handle);

	graphicsMeshInstance->gl.handle = handle;

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlGraphicsMesh(
			graphicsMeshInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*graphicsMesh = graphicsMeshInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void drawGlGraphicsMesh(
	GraphicsPipeline graphicsPipeline,
	GraphicsMesh graphicsMesh)
{
	assert(graphicsPipeline);
	assert(graphicsMesh);

	glBindVertexArray(
		graphicsMesh->gl.handle);
	glBindBuffer(
		GL_ARRAY_BUFFER,
		graphicsMesh->gl.vertexBuffer->gl.handle);
	glBindBuffer(
		GL_ELEMENT_ARRAY_BUFFER,
		graphicsMesh->gl.indexBuffer->gl.handle);
	assertOpenGL();

	if (graphicsPipeline->gl.onUniformsSet)
		graphicsPipeline->gl.onUniformsSet(graphicsPipeline);

	IndexType drawIndex = graphicsMesh->gl.indexType;

	GLenum glDrawIndex;
	size_t glIndexOffset;

	if (drawIndex == UINT16_INDEX_TYPE)
	{
		glDrawIndex = GL_UNSIGNED_SHORT;
		glIndexOffset = graphicsMesh->gl.indexOffset * sizeof(uint16_t);
	}
	else if (drawIndex == UINT32_INDEX_TYPE)
	{
		glDrawIndex = GL_UNSIGNED_INT;
		glIndexOffset = graphicsMesh->gl.indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	glDrawElements(
		graphicsPipeline->gl.drawMode,
		(GLsizei)graphicsMesh->gl.indexCount,
		glDrawIndex,
		(const void*)glIndexOffset);
	assertOpenGL();
}
#endif
