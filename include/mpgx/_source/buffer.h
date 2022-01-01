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
	size_t size;
	bool isConstant;
	bool isMapped;
} BaseBuffer_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkBuffer_T
{
	Window window;
	BufferType type;
	size_t size;
	bool isConstant;
	bool isMapped;
	bool writeAccess;
	VkBuffer handle;
	VmaAllocation allocation;
} VkBuffer_T;
#endif
#if MPGX_SUPPORT_OPENGL
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
	bool readAccess,
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

	if (readAccess == true)
	{
		vkResult = vmaInvalidateAllocation(
			allocator,
			allocation,
			0,
			VK_WHOLE_SIZE);

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
inline static void unmapVkBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	bool writeAccess)
{
	if (writeAccess == true)
	{
		VkResult vkResult = vmaFlushAllocation(
			allocator,
			allocation,
			0,
			VK_WHOLE_SIZE);

		if (vkResult != VK_SUCCESS)
			abort();
	}

	vmaUnmapMemory(
		allocator,
		allocation);
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

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	vmaUnmapMemory(
		allocator,
		allocation);
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
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	VkBuffer* stagingBuffer,
	VmaAllocation* stagingAllocation,
	size_t* stagingSize,
	VkBufferUsageFlags vkUsage,
	Window window,
	BufferType type,
	const void* data,
	size_t size,
	bool isConstant,
	bool useRayTracing,
	Buffer* buffer)
{
	Buffer bufferInstance = calloc(1, sizeof(Buffer_T));

	if (bufferInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	bufferInstance->vk.window = window;
	bufferInstance->vk.type = type;
	bufferInstance->vk.size = size;
	bufferInstance->vk.isConstant = isConstant;
	bufferInstance->vk.isMapped = false;
	bufferInstance->vk.writeAccess = false;

	VkBufferUsageFlags usageFlags = vkUsage;

	switch (type)
	{
	default:
		destroyVkBuffer(
			allocator,
			bufferInstance);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	case VERTEX_BUFFER_TYPE:
		usageFlags |= VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case INDEX_BUFFER_TYPE:
		usageFlags |= VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case UNIFORM_BUFFER_TYPE:
		usageFlags |= VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	case STORAGE_BUFFER_TYPE:
		usageFlags |= VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
		break;
	}

	if (data != NULL)
		usageFlags |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	// TODO: should we move this option to constructor?
	if (useRayTracing == true)
	{
		usageFlags |= VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
			VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR;
	}

	VkBufferCreateInfo bufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		size,
		usageFlags,
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
		if (isConstant == true && isGpuIntegrated == false)
		{
			bufferCreateInfo.usage = VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

			if (size > *stagingSize)
			{
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

			VkCommandBuffer commandBuffer;

			MpgxResult mpgxResult = allocateBeginVkOneTimeCommandBuffer(
				device,
				transferCommandPool,
				&commandBuffer);

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
				commandBuffer,
				*stagingBuffer,
				handle,
				1,
				&bufferCopy);

			mpgxResult = endSubmitWaitFreeVkCommandBuffer(
				device,
				transferQueue,
				transferCommandPool,
				transferFence,
				commandBuffer);

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
	size_t size,
	bool readAccess,
	bool writeAccess,
	void** map)
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

	if (mappedData == NULL)
		return FAILED_TO_MAP_MEMORY_MPGX_RESULT;

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
		return UNKNOWN_ERROR_MPGX_RESULT;

	*map = mappedData;
	return SUCCESS_MPGX_RESULT;
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
	const void* data,
	size_t size,
	bool isConstant,
	Buffer* buffer)
{
	Buffer bufferInstance = calloc(1, sizeof(Buffer_T));

	if (bufferInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	bufferInstance->gl.window = window;
	bufferInstance->gl.type = type;
	bufferInstance->gl.size = size;
	bufferInstance->gl.isConstant = isConstant;
	bufferInstance->gl.isMapped = false;

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
