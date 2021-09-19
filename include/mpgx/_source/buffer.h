#pragma once

#if MPGX_SUPPORT_VULKAN
#include "vk_mem_alloc.h"
#endif

#include <string.h>

typedef struct _VkBuffer
{
	Window window;
	uint8_t type;
	size_t size;
	bool isConstant;
#if MPGX_SUPPORT_VULKAN
	VkBuffer handle;
	VmaAllocation allocation;
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

#if MPGX_SUPPORT_VULKAN
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

		memcpy(
			mappedData + offset,
			data,
			size);

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
#endif

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

#if MPGX_SUPPORT_VULKAN
inline static Buffer createVkBuffer(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkImageUsageFlags _vkUsage,
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

	VkBufferUsageFlags vkUsage;

	switch (type)
	{
	default:
		free(buffer);
		return NULL;
	case VERTEX_BUFFER_TYPE:
		vkUsage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
		break;
	case INDEX_BUFFER_TYPE:
		vkUsage = VK_BUFFER_USAGE_INDEX_BUFFER_BIT;
		break;
	case UNIFORM_BUFFER_TYPE:
		vkUsage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
		break;
	}

	VkBufferUsageFlagBits usage = vkUsage | _vkUsage;

	if (data != NULL)
		usage |= VK_BUFFER_USAGE_TRANSFER_DST_BIT;

	VkBufferCreateInfo bufferCreateInfo = {
		VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
		NULL,
		0,
		size,
		usage,
		VK_SHARING_MODE_EXCLUSIVE,
		0,
		NULL,
	};

	VmaMemoryUsage vmaUsage;

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

	if (isConstant == true)
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	else
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

	VkBuffer handle;
	VmaAllocation allocation;

	VkResult result = vmaCreateBuffer(
		allocator,
		&bufferCreateInfo,
		&allocationCreateInfo,
		&handle,
		&allocation,
		NULL);

	if (result != VK_SUCCESS)
	{
		free(buffer);
		return NULL;
	}

	// TODO: possibly optimize with stage buffer caching
	if (isConstant == true && data != NULL)
	{
		bufferCreateInfo.usage = vkUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		result = vmaCreateBuffer(
			allocator,
			&bufferCreateInfo,
			&allocationCreateInfo,
			&stagingBuffer,
			&stagingAllocation,
			NULL);

		if (result != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		setVkBufferData(
			allocator,
			stagingAllocation,
			data,
			size,
			0);

		VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
			NULL,
			transferCommandPool,
			VK_COMMAND_BUFFER_LEVEL_PRIMARY,
			1,
		};

		VkCommandBuffer commandBuffer;

		result = vkAllocateCommandBuffers(
			device,
			&commandBufferAllocateInfo,
			&commandBuffer);

		if (result != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		VkCommandBufferBeginInfo commandBufferBeginInfo = {
			VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
			NULL,
			VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
			NULL,
		};

		result = vkBeginCommandBuffer(
			commandBuffer,
			&commandBufferBeginInfo);

		if (result != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		VkBufferCopy bufferCopy = {
			0,
			0,
			size,
		};

		vkCmdCopyBuffer(
			commandBuffer,
			stagingBuffer,
			handle,
			1,
			&bufferCopy);

		result = vkEndCommandBuffer(commandBuffer);

		if (result != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		VkSubmitInfo submitInfo = {
			VK_STRUCTURE_TYPE_SUBMIT_INFO,
			NULL,
			0,
			NULL,
			NULL,
			1,
			&commandBuffer,
			0,
			NULL,
		};

		result = vkQueueSubmit(
			transferQueue,
			1,
			&submitInfo,
			NULL);

		if (result != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		result = vkQueueWaitIdle(transferQueue);

		if (result != VK_SUCCESS)
		{
			vkFreeCommandBuffers(
				device,
				transferCommandPool,
				1,
				&commandBuffer);
			vmaDestroyBuffer(
				allocator,
				stagingBuffer,
				stagingAllocation);
			vmaDestroyBuffer(
				allocator,
				handle,
				allocation);
			free(buffer);
			return NULL;
		}

		vkFreeCommandBuffers(
			device,
			transferCommandPool,
			1,
			&commandBuffer);
		vmaDestroyBuffer(
			allocator,
			stagingBuffer,
			stagingAllocation);
	}

	buffer->vk.window = window;
	buffer->vk.type = type;
	buffer->vk.isConstant = isConstant;
	buffer->vk.handle = handle;
	buffer->vk.allocation = allocation;
	return buffer;
}
#endif

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

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkBuffer(
	VmaAllocator allocator,
	Buffer buffer)
{
	vmaDestroyBuffer(
		allocator,
		buffer->vk.handle,
		buffer->vk.allocation);
}
#endif

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

// TODO: image data set (demo_set_image_layout) from cube.c
