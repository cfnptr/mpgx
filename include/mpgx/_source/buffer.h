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
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/opengl.h"

#include <string.h>

typedef struct BaseBuffer_T
{
	Window window;
	BufferType type;
	size_t size;
	bool isConstant;
	bool isMapped;
} BaseBuffer_T;
typedef struct VkBuffer_T
{
	Window window;
	BufferType type;
	size_t size;
	bool isConstant;
	bool isMapped;
#if MPGX_SUPPORT_VULKAN
	bool writeAccess;
	VkBuffer handle;
	VmaAllocation allocation;
#endif
} VkBuffer_T;
typedef struct GlBuffer_T
{
	Window window;
	BufferType type;
	size_t size;
	bool isConstant;
	bool isMapped;
	GLenum glType;
	GLuint handle;
} GlBuffer_T;
union Buffer_T
{
	BaseBuffer_T base;
	VkBuffer_T vk;
	GlBuffer_T gl;
};

#if MPGX_SUPPORT_VULKAN
inline static void* mapVkBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	bool readAccess)
{
	void* mappedData;

	VkResult result = vmaMapMemory(
		allocator,
		allocation,
		&mappedData);

	if (result != VK_SUCCESS)
		return NULL;

	if (readAccess == true)
	{
		result = vmaInvalidateAllocation(
			allocator,
			allocation,
			0,
			VK_WHOLE_SIZE);

		if (result != VK_SUCCESS)
		{
			vmaUnmapMemory(
				allocator,
				allocation);
			return NULL;
		}
	}

	return mappedData;
}
inline static void unmapVkBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	bool writeAccess)
{
	if (writeAccess == true)
	{
		VkResult result = vmaFlushAllocation(
			allocator,
			allocation,
			0,
			VK_WHOLE_SIZE);

		if (result != VK_SUCCESS)
			abort();
	}

	vmaUnmapMemory(
		allocator,
		allocation);
}

inline static void setVkBufferData(
	VmaAllocator allocator,
	VmaAllocation allocation,
	const void* data,
	size_t size,
	size_t offset)
{
	void* mappedData;

	VkResult result = vmaMapMemory(
		allocator,
		allocation,
		&mappedData);

	if (result != VK_SUCCESS)
		abort();

	uint8_t* _mappedData = mappedData;
	memcpy(_mappedData + offset, data, size);

	result = vmaFlushAllocation(
		allocator,
		allocation,
		offset,
		size);

	if (result != VK_SUCCESS)
		abort();

	vmaUnmapMemory(
		allocator,
		allocation);
}

inline static void destroyVkBuffer(
	VmaAllocator allocator,
	Buffer buffer)
{
	vmaDestroyBuffer(
		allocator,
		buffer->vk.handle,
		buffer->vk.allocation);
	free(buffer);
}
inline static Buffer createVkBuffer(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	VkBuffer* _stagingBuffer,
	VmaAllocation* _stagingAllocation,
	size_t* _stagingSize,
	VkBufferUsageFlags _vkUsage,
	Window window,
	BufferType type,
	const void* data,
	size_t size,
	bool isConstant)
{
	Buffer buffer = calloc(1, sizeof(Buffer_T));

	if (buffer == NULL)
		return NULL;

	buffer->vk.window = window;
	buffer->vk.type = type;
	buffer->vk.size = size;
	buffer->vk.isConstant = isConstant;
	buffer->vk.isMapped = false;
	buffer->vk.writeAccess = false;

	VkBufferUsageFlags vkUsage = _vkUsage;

	switch (type)
	{
	default:
		abort();
	case VERTEX_BUFFER_TYPE:
		vkUsage |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case INDEX_BUFFER_TYPE:
		vkUsage |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case UNIFORM_BUFFER_TYPE:
		vkUsage |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case STORAGE_BUFFER_TYPE:
		vkUsage |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	if (data != NULL)
		vkUsage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo bufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		size,
		vkUsage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
	};

	VmaAllocationCreateInfo allocationCreateInfo;
	memset(&allocationCreateInfo, 0, sizeof(VmaAllocationCreateInfo));

	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

	bool isGpuIntegrated = isVkGpuIntegrated(window);

	if (isConstant == true && isGpuIntegrated == false)
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	else
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBuffer handle;
	VmaAllocation allocation;

	VkResult vkResult = vmaCreateBuffer(
		allocator,
		&bufferCreateInfo,
		&allocationCreateInfo,
		&handle,
		&allocation,
		NULL);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkBuffer(
			allocator,
			buffer);
		return NULL;
	}

	buffer->vk.handle = handle;
	buffer->vk.allocation = allocation;

	if (data != NULL)
	{
		if (isConstant == true && isGpuIntegrated == false)
		{
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			if (size > *_stagingSize)
			{
				VkBuffer stagingBuffer;
				VmaAllocation stagingAllocation;

				vkResult = vmaCreateBuffer(
					allocator,
					&bufferCreateInfo,
					&allocationCreateInfo,
					&stagingBuffer,
					&stagingAllocation,
					NULL);

				if (vkResult != VK_SUCCESS)
				{
					destroyVkBuffer(
						allocator,
						buffer);
					return NULL;
				}

				vmaDestroyBuffer(
					allocator,
					*_stagingBuffer,
					*_stagingAllocation);

				*_stagingBuffer = stagingBuffer;
				*_stagingAllocation = stagingAllocation;
				*_stagingSize = size;
			}

			setVkBufferData(
				allocator,
				*_stagingAllocation,
				data,
				size,
				0);

			VkCommandBuffer commandBuffer = allocateBeginVkOneTimeCommandBuffer(
				device, transferCommandPool);

			if (commandBuffer == NULL)
			{
				destroyVkBuffer(
					allocator,
					buffer);
				return NULL;
			}

			VkBufferCopy bufferCopy = {
				0,
				0,
				size,
			};

			vkCmdCopyBuffer(
				commandBuffer,
				*_stagingBuffer,
				handle,
				1,
				&bufferCopy);

			bool result = endSubmitWaitFreeVkCommandBuffer(
				device,
				transferQueue,
				transferCommandPool,
				transferFence,
				commandBuffer);

			if (result == false)
			{
				destroyVkBuffer(
					allocator,
					buffer);
				return NULL;
			}
		}
		else
		{
			setVkBufferData(
				allocator,
				allocation,
				data,
				size,
				0);
		}
	}

	return buffer;
}
#endif

inline static void* mapGlBuffer(
	GLenum type,
	GLuint handle,
	size_t size,
	bool readAccess,
	bool writeAccess)
{
	glBindBuffer(
		type,
		handle);

	GLbitfield glAccess = 0;

	if (readAccess == true)
		glAccess |= GL_MAP_READ_BIT;
	if (writeAccess == true)
		glAccess |= GL_MAP_WRITE_BIT;

	void* mappedData = glMapBufferRange(
		type,
		0,
		(GLsizeiptr)size,
		glAccess);

	assertOpenGL();
	return mappedData;
}
inline static void unmapGlBuffer(
	GLenum type,
	GLuint handle)
{
	glBindBuffer(
		type,
		handle);
	glUnmapBuffer(type);
	assertOpenGL();
}

inline static void setGlBufferData(
	GLenum type,
	GLuint buffer,
	const void* data,
	size_t size,
	size_t offset)
{
	glBindBuffer(
		type,
		buffer);
	glBufferSubData(
		type,
		(GLintptr)offset,
		(GLsizeiptr)size,
		data);
	assertOpenGL();
}

inline static void destroyGlBuffer(
	Buffer buffer)
{
	if (buffer == NULL)
		return;

	makeWindowContextCurrent(
		buffer->gl.window);

	glDeleteBuffers(
		GL_ONE,
		&buffer->gl.handle);
	assertOpenGL();

	free(buffer);
}
inline static Buffer createGlBuffer(
	Window window,
	BufferType type,
	const void* data,
	size_t size,
	bool isConstant)
{
	Buffer buffer = calloc(1, sizeof(Buffer_T));

	if (buffer == NULL)
		return NULL;

	buffer->gl.window = window;
	buffer->gl.type = type;
	buffer->gl.size = size;
	buffer->gl.isConstant = isConstant;
	buffer->gl.isMapped = false;

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
		destroyGlBuffer(buffer);
		return NULL;
	}

	buffer->gl.glType = glType;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenBuffers(
		GL_ONE,
		&handle);

	buffer->gl.handle = handle;

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

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		glDeleteBuffers(
			GL_ONE,
			&handle);
		destroyGlBuffer(buffer);
		return NULL;
	}

	return buffer;
}
