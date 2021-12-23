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
#include "mpgx/defines.h"

#if MPGX_SUPPORT_VULKAN
#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

inline static VkCommandBuffer allocateBeginVkOneTimeCommandBuffer(
	VkDevice device,
	VkCommandPool commandPool)
{
	VkCommandBufferAllocateInfo commandBufferAllocateInfo = {
		VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
		NULL,
		commandPool,
		VK_COMMAND_BUFFER_LEVEL_PRIMARY,
		1,
	};

	VkCommandBuffer commandBuffer;

	VkResult vkResult = vkAllocateCommandBuffers(
		device,
		&commandBufferAllocateInfo,
		&commandBuffer);

	if (vkResult != VK_SUCCESS)
		return NULL;

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
			commandPool,
			1,
			&commandBuffer);
		return NULL;
	}

	return commandBuffer;
}
inline static bool endSubmitWaitFreeVkCommandBuffer(
	VkDevice device,
	VkQueue queue,
	VkCommandPool commandPool,
	VkFence fence,
	VkCommandBuffer commandBuffer)
{
	VkResult vkResult = vkEndCommandBuffer(commandBuffer);

	if (vkResult != VK_SUCCESS)
	{
		vkFreeCommandBuffers(
			device,
			commandPool,
			1,
			&commandBuffer);
		return false;
	}

	vkResult = vkResetFences(
		device,
		1,
		&fence);

	if (vkResult != VK_SUCCESS)
	{
		vkFreeCommandBuffers(
			device,
			commandPool,
			1,
			&commandBuffer);
		return false;
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
		vkFreeCommandBuffers(
			device,
			commandPool,
			1,
			&commandBuffer);
		return false;
	}

	vkResult = vkWaitForFences(
		device,
		1,
		&fence,
		VK_TRUE,
		UINT64_MAX);

	vkFreeCommandBuffers(
		device,
		commandPool,
		1,
		&commandBuffer);

	if (vkResult != VK_SUCCESS)
		return false;

	return true;
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
