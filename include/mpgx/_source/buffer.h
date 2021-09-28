#pragma once
#include "mpgx/_source/opengl.h"

#if MPGX_SUPPORT_VULKAN
#include "mpgx/_source/vulkan.h"
#endif

// TODO: possibly add buffer map/unmap functions
// https://github.com/InjectorGames/InjectorEngine/blob/master/Source/Graphics/Vulkan/VkGpuBuffer.cpp

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
inline static bool setVkBufferData(
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
		return false;

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
		return false;

	vmaUnmapMemory(
		allocator,
		allocation);
	return true;
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
	VkBufferUsageFlags _vkUsage,
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

	bool isGpuIntegrated = isVkGpuIntegrated(window);

	VmaAllocationCreateInfo allocationCreateInfo = {};
	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;
	// TODO: VMA_MEMORY_USAGE_GPU_LAZILY_ALLOCATED on mobiles

	// TODO: investigate if this solution optimal

	if (isGpuIntegrated == true)
	{
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;
	}
	else
	{
		if (isConstant == true)
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		else
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
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
		free(buffer);
		return NULL;
	}

	if (data != NULL)
	{
		if (isConstant == true && isGpuIntegrated == false)
		{
			bufferCreateInfo.usage = vkUsage | VK_BUFFER_USAGE_TRANSFER_SRC_BIT;
			allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

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
				vmaDestroyBuffer(
					allocator,
					handle,
					allocation);
				free(buffer);
				return NULL;
			}

			bool result = setVkBufferData(
				allocator,
				stagingAllocation,
				data,
				size,
				0);

			if (result == false)
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

			VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
				VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
				NULL,
				transferCommandPool,
				VK_COMMAND_BUFFER_LEVEL_PRIMARY,
				1,
			};

			VkCommandBuffer commandBuffer;

			vkResult = vkAllocateCommandBuffers(
				device,
				&commandBufferAllocateInfo,
				&commandBuffer);

			if (vkResult != VK_SUCCESS)
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

			vkResult = vkBeginCommandBuffer(
				commandBuffer,
				&commandBufferBeginInfo);

			if (vkResult != VK_SUCCESS)
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

			vkResult = vkEndCommandBuffer(commandBuffer);

			if (vkResult != VK_SUCCESS)
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

			vkResult = vkQueueSubmit(
				transferQueue,
				1,
				&submitInfo,
				NULL);

			if (vkResult != VK_SUCCESS)
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

			vkResult = vkQueueWaitIdle(transferQueue);

			if (vkResult != VK_SUCCESS)
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
		else
		{
			bool result = setVkBufferData(
				allocator,
				allocation,
				data,
				size,
				0);

			if (result == false)
			{
				vmaDestroyBuffer(
					allocator,
					handle,
					allocation);
				free(buffer);
				return NULL;
			}
		}
	}

	buffer->vk.window = window;
	buffer->vk.type = type;
	buffer->vk.size = size;
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

	GLenum error = glGetError();

	if (error != GL_NO_ERROR)
	{
		glDeleteBuffers(
			GL_ONE,
			&handle);
		free(buffer);
		return NULL;
	}

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
	free(buffer);
}
#endif

inline static void destroyGlBuffer(
	Buffer buffer)
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
