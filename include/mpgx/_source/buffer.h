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
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/opengl.h"

#include <string.h>

typedef struct BaseBuffer_T
{
	Window window;
	BufferType type;
	BufferUsage usage;
	BufferFlag flags;
	size_t size;
#ifndef NDEBUG
	bool isMapped;
#endif
} BaseBuffer_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkBuffer_T
{
	Window window;
	BufferType type;
	BufferUsage usage;
	BufferFlag flags;
	size_t size;
#ifndef NDEBUG
	bool isMapped;
#endif
	size_t mapSize;
	size_t mapOffset;
	VkBuffer handle;
	VmaAllocation allocation;
} VkBuffer_T;
#endif
#if MPGX_SUPPORT_OPENGL
typedef struct GlBuffer_T
{
	Window window;
	BufferType type;
	BufferUsage usage;
	BufferFlag flags;
	size_t size;
#ifndef NDEBUG
	bool isMapped;
#endif
	GLenum glType;
	GLuint handle;
} GlBuffer_T;
#endif
union Buffer_T
{
	BaseBuffer_T base;
#if MPGX_SUPPORT_VULKAN
	VkBuffer_T vk;
#endif
#if MPGX_SUPPORT_OPENGL
	GlBuffer_T gl;
#endif
};

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult mapVkBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	BufferUsage usage,
	size_t size,
	size_t offset,
	void** map)
{
	void* mappedData;

	VkResult vkResult = vmaMapMemory(
		allocator,
		allocation,
		&mappedData);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_MEMORY_MAP_FAILED)
			return FAILED_TO_MAP_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	if (usage == GPU_TO_CPU_BUFFER_USAGE)
	{
		vkResult = vmaInvalidateAllocation(
			allocator,
			allocation,
			offset,
			size);

		if (vkResult != VK_SUCCESS)
		{
			vmaUnmapMemory(
				allocator,
				allocation);

			if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			else
				return UNKNOWN_ERROR_MPGX_RESULT;
		}
	}

	*map = mappedData;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult unmapVkBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	BufferUsage usage,
	size_t size,
	size_t offset)
{
	if (usage == CPU_ONLY_BUFFER_USAGE ||
		usage == CPU_TO_GPU_BUFFER_USAGE)
	{
		VkResult vkResult = vmaFlushAllocation(
			allocator,
			allocation,
			offset,
			size);

		if (vkResult != VK_SUCCESS)
		{
			vmaUnmapMemory(
				allocator,
				allocation);

			if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			else
				return UNKNOWN_ERROR_MPGX_RESULT;
		}
	}

	vmaUnmapMemory(
		allocator,
		allocation);
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult setVkBufferData(
	VmaAllocator allocator,
	VmaAllocation allocation,
	const void* data,
	size_t size,
	size_t offset)
{
	void* mappedData;

	VkResult vkResult = vmaMapMemory(
		allocator,
		allocation,
		&mappedData);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_MEMORY_MAP_FAILED)
			return FAILED_TO_MAP_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	uint8_t* mappedBytes = mappedData;
	memcpy(mappedBytes + offset, data, size);

	vkResult = vmaFlushAllocation(
		allocator,
		allocation,
		offset,
		size);

	vmaUnmapMemory(
		allocator,
		allocation);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	return SUCCESS_MPGX_RESULT;
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
inline static MpgxResult createVkBuffer(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	VkBuffer* stagingBuffer,
	VmaAllocation* stagingAllocation,
	size_t* stagingSize,
	VkBufferUsageFlags vkUsage,
	Window window,
	BufferType type,
	BufferUsage usage,
	BufferFlag flags,
	const void* data,
	size_t size,
	bool useRayTracing,
	Buffer* buffer)
{
	Buffer bufferInstance = calloc(1, sizeof(Buffer_T));

	if (bufferInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	bufferInstance->vk.window = window;
	bufferInstance->vk.type = type;
	bufferInstance->vk.size = size;
#ifndef NDEBUG
	bufferInstance->vk.isMapped = false;
#endif
	bufferInstance->vk.mapSize = 0;
	bufferInstance->vk.mapOffset = 0;

	bool isIntegrated = isVkDeviceIntegrated(window);

	if (isIntegrated == true)
		usage = CPU_ONLY_BUFFER_USAGE;

	bufferInstance->vk.usage = usage;

	switch (type)
	{
	default:
		destroyVkBuffer(
			allocator,
			bufferInstance);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
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

	if (data != NULL && usage == GPU_ONLY_BUFFER_USAGE)
		flags |= TRANSFER_DESTINATION_BUFFER_FLAG;

	bufferInstance->vk.flags = flags;

	// TODO: move this to the usage
	if (useRayTracing == true)
	{
		vkUsage |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	if (flags & TRANSFER_SOURCE_BUFFER_FLAG)
		vkUsage |= VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
	if (flags & TRANSFER_DESTINATION_BUFFER_FLAG)
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

	switch (usage)
	{
	default:
		destroyVkBuffer(
			allocator,
			bufferInstance);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	case CPU_ONLY_BUFFER_USAGE:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
		break;
	case GPU_ONLY_BUFFER_USAGE:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		break;
	case CPU_TO_GPU_BUFFER_USAGE:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
		break;
	case GPU_TO_CPU_BUFFER_USAGE:
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_TO_CPU;
		break;
	}

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
			bufferInstance);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	bufferInstance->vk.handle = handle;
	bufferInstance->vk.allocation = allocation;

	if (data != NULL)
	{
		VmaAllocationInfo allocationInfo;

		vmaGetAllocationInfo(
			allocator,
			allocation,
			&allocationInfo);

		VkMemoryPropertyFlags memoryPropertyFlags;

		vmaGetMemoryTypeProperties(
			allocator,
			allocationInfo.memoryType,
			&memoryPropertyFlags);

		if ((memoryPropertyFlags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT) == 0)
		{
			if (size > *stagingSize)
			{
				bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
				allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

				VkBuffer stagingBufferInstance;
				VmaAllocation stagingAllocationInstance;

				vkResult = vmaCreateBuffer(
					allocator,
					&bufferCreateInfo,
					&allocationCreateInfo,
					&stagingBufferInstance,
					&stagingAllocationInstance,
					NULL);

				if (vkResult != VK_SUCCESS)
				{
					destroyVkBuffer(
						allocator,
						bufferInstance);

					if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
						return OUT_OF_HOST_MEMORY_MPGX_RESULT;
					else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
						return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
					else
						return UNKNOWN_ERROR_MPGX_RESULT;
				}

				vmaDestroyBuffer(
					allocator,
					*stagingBuffer,
					*stagingAllocation);

				*stagingBuffer = stagingBufferInstance;
				*stagingAllocation = stagingAllocationInstance;
				*stagingSize = size;
			}

			setVkBufferData(
				allocator,
				*stagingAllocation,
				data,
				size,
				0);

			MpgxResult mpgxResult = beginVkOneTimeCommandBuffer(
				transferCommandBuffer);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyVkBuffer(
					allocator,
					bufferInstance);
				return mpgxResult;
			}

			VkBufferCopy bufferCopy = {
				0,
				0,
				size,
			};

			vkCmdCopyBuffer(
				transferCommandBuffer,
				*stagingBuffer,
				handle,
				1,
				&bufferCopy);

			mpgxResult = endSubmitWaitVkCommandBuffer(
				device,
				transferQueue,
				transferFence,
				transferCommandBuffer);

			if (mpgxResult != SUCCESS_MPGX_RESULT)
			{
				destroyVkBuffer(
					allocator,
					bufferInstance);
				return mpgxResult;
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

	*buffer = bufferInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

#if MPGX_SUPPORT_OPENGL
inline static MpgxResult mapGlBuffer(
	GLenum type,
	GLuint handle,
	BufferType usage,
	size_t size,
	size_t offset,
	void** map)
{
	glBindBuffer(
		type,
		handle);

	GLbitfield glAccess = 0;

	if (usage == GPU_TO_CPU_BUFFER_USAGE)
		glAccess |= GL_MAP_READ_BIT;
	else if (usage == CPU_TO_GPU_BUFFER_USAGE)
		glAccess |= GL_MAP_WRITE_BIT;

	void* mappedData = glMapBufferRange(
		type,
		(GLintptr)offset,
		(GLsizeiptr)size,
		glAccess);

	if (mappedData == NULL)
		return FAILED_TO_MAP_MEMORY_MPGX_RESULT;

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		return UNKNOWN_ERROR_MPGX_RESULT;

	*map = mappedData;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult unmapGlBuffer(
	GLenum type,
	GLuint handle)
{
	glBindBuffer(
		type,
		handle);
	glUnmapBuffer(type);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		return UNKNOWN_ERROR_MPGX_RESULT;

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult setGlBufferData(
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

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		return UNKNOWN_ERROR_MPGX_RESULT;

	return SUCCESS_MPGX_RESULT;
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
inline static MpgxResult createGlBuffer(
	Window window,
	BufferType type,
	BufferUsage usage,
	BufferFlag flags,
	const void* data,
	size_t size,
	Buffer* buffer)
{
	Buffer bufferInstance = calloc(1, sizeof(Buffer_T));

	if (bufferInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	bufferInstance->gl.window = window;
	bufferInstance->gl.type = type;
	bufferInstance->gl.usage = usage;
	bufferInstance->gl.flags = flags;
	bufferInstance->gl.size = size;
#ifndef NDEBUG
	bufferInstance->gl.isMapped = false;
#endif

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
		destroyGlBuffer(bufferInstance);
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	}

	bufferInstance->gl.glType = glType;

	makeWindowContextCurrent(window);

	GLuint handle = GL_ZERO;

	glGenBuffers(
		GL_ONE,
		&handle);

	bufferInstance->gl.handle = handle;

	GLenum glUsage;

	switch (usage)
	{
	default:
		destroyGlBuffer(bufferInstance);
		return OPENGL_IS_NOT_SUPPORTED_MPGX_RESULT;
	case CPU_ONLY_BUFFER_USAGE:
		glUsage = GL_STATIC_COPY;
		break;
	case GPU_ONLY_BUFFER_USAGE:
		glUsage = GL_STATIC_DRAW;
		break;
	case CPU_TO_GPU_BUFFER_USAGE:
		glUsage = GL_STREAM_DRAW;
		break;
	case GPU_TO_CPU_BUFFER_USAGE:
		glUsage = GL_STREAM_READ;
		break;
	}

	glBindBuffer(
		glType,
		handle);
	glBufferData(
		glType,
		(GLsizeiptr)(size),
		data,
		glUsage);

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		destroyGlBuffer(bufferInstance);

		if (error == GL_OUT_OF_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*buffer = bufferInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
