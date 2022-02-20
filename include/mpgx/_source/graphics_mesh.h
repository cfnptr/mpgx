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
	uint8_t _alignment[3];
	VkIndexType vkIndexType;
	VkDeviceSize vkIndexOffset;
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
	GLenum glIndexType;
	size_t glIndexOffset;
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

	if (indexType == UINT16_INDEX_TYPE)
	{
		graphicsMeshInstance->vk.vkIndexType = VK_INDEX_TYPE_UINT16;
		graphicsMeshInstance->vk.vkIndexOffset = indexOffset * sizeof(uint16_t);
	}
	else if (indexType == UINT32_INDEX_TYPE)
	{
		graphicsMeshInstance->vk.vkIndexType = VK_INDEX_TYPE_UINT32;
		graphicsMeshInstance->vk.vkIndexOffset = indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

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
	GraphicsPipeline graphicsPipeline,
	GraphicsMesh graphicsMesh)
{
	assert(commandBuffer);
	assert(graphicsMesh);

	if (graphicsPipeline->base.onUniformsSet)
		graphicsPipeline->base.onUniformsSet(graphicsPipeline);

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
		graphicsMesh->vk.vkIndexOffset,
		graphicsMesh->vk.vkIndexType);
	vkCmdDrawIndexed(
		commandBuffer,
		(uint32_t)graphicsMesh->vk.indexCount,
		1,
		0,
		0,
		0);
}
inline static void setVkGraphicsMeshIndexType(
	GraphicsMesh graphicsMesh,
	IndexType indexType)
{
	if (indexType == UINT16_INDEX_TYPE)
		graphicsMesh->vk.vkIndexType = VK_INDEX_TYPE_UINT16;
	else if (indexType == UINT32_INDEX_TYPE)
		graphicsMesh->vk.vkIndexType = VK_INDEX_TYPE_UINT32;
	else
		abort();

	graphicsMesh->vk.indexType = indexType;
}
inline static void setVkGraphicsMeshIndexOffset(
	GraphicsMesh graphicsMesh,
	size_t indexOffset)
{
	if (graphicsMesh->vk.indexType == UINT16_INDEX_TYPE)
		graphicsMesh->vk.vkIndexOffset = indexOffset * sizeof(uint16_t);
	else if (graphicsMesh->vk.indexType == UINT32_INDEX_TYPE)
		graphicsMesh->vk.vkIndexOffset = indexOffset * sizeof(uint32_t);
	else
		abort();

	graphicsMesh->vk.indexOffset = indexOffset;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static void destroyGlGraphicsMesh(
	GraphicsMesh graphicsMesh,
	bool destroyBuffers)
{
	if (!graphicsMesh)
		return;

	makeGlWindowContextCurrent(
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

	if (indexType == UINT16_INDEX_TYPE)
	{
		graphicsMeshInstance->gl.glIndexType = GL_UNSIGNED_SHORT;
		graphicsMeshInstance->gl.glIndexOffset = indexOffset * sizeof(uint16_t);
	}
	else if (indexType == UINT32_INDEX_TYPE)
	{
		graphicsMeshInstance->gl.glIndexType = GL_UNSIGNED_INT;
		graphicsMeshInstance->gl.glIndexOffset = indexOffset * sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	makeGlWindowContextCurrent(window);

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

	glDrawElements(
		graphicsPipeline->gl.drawMode,
		(GLsizei)graphicsMesh->gl.indexCount,
		graphicsMesh->gl.glIndexType,
		(const void*)graphicsMesh->gl.glIndexOffset);
	assertOpenGL();
}
inline static void setGlGraphicsMeshIndexType(
	GraphicsMesh graphicsMesh,
	IndexType indexType)
{
	if (indexType == UINT16_INDEX_TYPE)
		graphicsMesh->gl.glIndexType = GL_UNSIGNED_SHORT;
	else if (indexType == UINT32_INDEX_TYPE)
		graphicsMesh->gl.glIndexType = GL_UNSIGNED_INT;
	else
		abort();

	graphicsMesh->gl.indexType = indexType;
}
inline static void setGlGraphicsMeshIndexOffset(
	GraphicsMesh graphicsMesh,
	size_t indexOffset)
{
	if (graphicsMesh->gl.indexType == UINT16_INDEX_TYPE)
		graphicsMesh->gl.glIndexOffset = indexOffset * sizeof(uint16_t);
	else if (graphicsMesh->gl.indexType == UINT32_INDEX_TYPE)
		graphicsMesh->gl.glIndexOffset = indexOffset * sizeof(uint32_t);
	else
		abort();

	graphicsMesh->gl.indexOffset = indexOffset;
}
#endif
