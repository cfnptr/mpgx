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
#include "mpgx/defines.h"

#if MPGX_SUPPORT_VULKAN
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

inline static MpgxResult beginVkOneTimeCommandBuffer(
	VkCommandBuffer commandBuffer)
{
	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
		NULL,
	};

	VkResult vkResult = vkBeginCommandBuffer(
		commandBuffer,
		&commandBufferBeginInfo);

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
inline static MpgxResult endSubmitWaitVkCommandBuffer(
	VkDevice device,
	VkQueue queue,
	VkFence fence,
	VkCommandBuffer commandBuffer)
{
	VkResult vkResult = vkEndCommandBuffer(commandBuffer);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	vkResult = vkResetFences(
		device,
		1,
		&fence);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
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
		queue,
		1,
		&submitInfo,
		fence);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_DEVICE_LOST)
			return DEVICE_IS_LOST_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	vkResult = vkWaitForFences(
		device,
		1,
		&fence,
		VK_TRUE,
		UINT64_MAX);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_DEVICE_LOST)
			return DEVICE_IS_LOST_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult allocateBeginVkCommandBuffer(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBuffer* commandBuffer)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	VkCommandBuffer commandBufferInstance;

	VkResult vkResult = vkAllocateCommandBuffers(
		device,
		&commandBufferAllocateInfo,
		&commandBufferInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkCommandBufferBeginInfo commandBufferBeginInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
		NULL,
		0,
		NULL,
	};

	vkResult = vkBeginCommandBuffer(
		commandBufferInstance,
		&commandBufferBeginInfo);

	if (vkResult != VK_SUCCESS)
	{
		vkFreeCommandBuffers(
			device,
			commandPool,
			1,
			&commandBufferInstance);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*commandBuffer = commandBufferInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult endVkCommandBuffer(
	VkCommandBuffer commandBuffer)
{
	VkResult vkResult = vkEndCommandBuffer(commandBuffer);

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
inline static MpgxResult submitVkCommandBuffer(
	VkDevice device,
	VkQueue queue,
	VkFence fence,
	VkCommandBuffer commandBuffer)
{
	if (fence != NULL)
	{
		VkResult vkResult = vkResetFences(
			device,
			1,
			&fence);

		if (vkResult != VK_SUCCESS)
		{
			if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			else
				return UNKNOWN_ERROR_MPGX_RESULT;
		}
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

	VkResult vkResult = vkQueueSubmit(
		queue,
		1,
		&submitInfo,
		fence);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_DEVICE_LOST)
			return DEVICE_IS_LOST_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkDescriptorSetLayout(
	VkDevice device,
	VkDescriptorSetLayoutBinding* descriptorSetLayoutBindings,
	uint32_t descriptorSetLayoutBindingCount,
	VkDescriptorSetLayout* descriptorSetLayout)
{
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		descriptorSetLayoutBindingCount,
		descriptorSetLayoutBindings,
	};

	VkDescriptorSetLayout descriptorSetLayoutInstance;

	VkResult vkResult = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayoutInstance);

	if(vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*descriptorSetLayout = descriptorSetLayoutInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult createVkDescriptorPool(
	VkDevice device,
	uint32_t maxSetCount,
	VkDescriptorPoolSize* descriptorPoolSizes,
	uint32_t descriptorPoolSizeCount,
	VkDescriptorPool* descriptorPool)
{
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		maxSetCount,
		descriptorPoolSizeCount,
		descriptorPoolSizes,
	};

	VkDescriptorPool descriptorPoolInstance;

	VkResult vkResult = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPoolInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*descriptorPool = descriptorPoolInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult allocateVkDescriptorSet(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	VkDescriptorSet* descriptorSet)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		1,
		&descriptorSetLayout,
	};

	VkDescriptorSet descriptorSetInstance;

	VkResult vkResult = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		&descriptorSetInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;

		// TODO: handle VK_ERROR_FRAGMENTED_POOL, VK_ERROR_OUT_OF_POOL_MEMORY
	}

	*descriptorSet = descriptorSetInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult allocateVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t descriptorSetCount,
	VkDescriptorSet** descriptorSets)
{
	VkDescriptorSetLayout* descriptorSetLayouts = malloc(
		descriptorSetCount * sizeof(VkDescriptorSetLayout));

	if (descriptorSetLayouts == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkDescriptorSet* descriptorSetArray = malloc(
		descriptorSetCount * sizeof(VkDescriptorSet));

	if (descriptorSetArray == NULL)
	{
		free(descriptorSetLayouts);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	for (uint32_t i = 0; i < descriptorSetCount; i++)
		descriptorSetLayouts[i] = descriptorSetLayout;

	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		descriptorSetCount,
		descriptorSetLayouts,
	};

	VkResult vkResult = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		descriptorSetArray);

	free(descriptorSetLayouts);

	if (vkResult != VK_SUCCESS)
	{
		free(descriptorSets);

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;

		// TODO: handle VK_ERROR_FRAGMENTED_POOL, VK_ERROR_OUT_OF_POOL_MEMORY
	}

	*descriptorSets = descriptorSetArray;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkFence(
	VkDevice device,
	VkFenceCreateFlags flags,
	VkFence* fence)
{
	VkFenceCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
		NULL,
		flags,
	};

	VkFence fenceInstance;

	VkResult vkResult = vkCreateFence(
		device,
		&createInfo,
		NULL,
		&fenceInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*fence = fenceInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult getVkFenceStatus(
	VkDevice device,
	VkFence fence,
	bool* status)
{
	VkResult vkResult = vkGetFenceStatus(
		device, fence);

	if (vkResult == VK_SUCCESS)
	{
		*status = true;
		return SUCCESS_MPGX_RESULT;
	}
	else if (vkResult == VK_NOT_READY)
	{
		*status = false;
		return SUCCESS_MPGX_RESULT;
	}
	else
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_DEVICE_LOST)
			return DEVICE_IS_LOST_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}
}

inline static MpgxResult createVkSemaphore(
	VkDevice device,
	VkSemaphore* semaphore)
{
	VkSemaphoreCreateInfo createInfo = {
		VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
		NULL,
		0,
	};

	VkSemaphore semaphoreInstance;

	VkResult vkResult = vkCreateSemaphore(
		device,
		&createInfo,
		NULL,
		&semaphoreInstance);

	if (vkResult != VK_SUCCESS)
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	*semaphore = semaphoreInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static bool getVkCompareOperator(
	CompareOperator compareOperator,
	VkCompareOp* vkCompareOperator)
{
	switch (compareOperator)
	{
	default:
		return false;
	case LESS_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_LESS;
		return true;
	case LESS_OR_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_LESS_OR_EQUAL;
		return true;
	case GREATER_OR_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_GREATER_OR_EQUAL;
		return true;
	case GREATER_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_GREATER;
		return true;
	case EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_EQUAL;
		return true;
	case NOT_EQUAL_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_NOT_EQUAL;
		return true;
	case ALWAYS_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_ALWAYS;
		return true;
	case NEVER_COMPARE_OPERATOR:
		*vkCompareOperator = VK_COMPARE_OP_NEVER;
		return true;
	}
}

static VkPhysicalDeviceProperties physicalDeviceProperties;

inline static const char* getVkWindowGpuName(
	VkPhysicalDevice physicalDevice)
{
	vkGetPhysicalDeviceProperties(
		physicalDevice,
		&physicalDeviceProperties);
	return physicalDeviceProperties.deviceName;
}
#endif
