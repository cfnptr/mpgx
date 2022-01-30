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

#if __APPLE__
#define VK_ENABLE_BETA_EXTENSIONS
#endif

#include "vulkan/vulkan.h"
#include "vk_mem_alloc.h"

#define alignVkMemory(size, alignment) \
	((size + (alignment - 1)) & ~(alignment - 1))

inline static MpgxResult vkToMpgxResult(VkResult vkResult)
{
	// TODO: handle other Vulkan results
	switch (vkResult)
	{
	default:
		return UNKNOWN_ERROR_MPGX_RESULT;
	case VK_ERROR_OUT_OF_HOST_MEMORY:
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	case VK_ERROR_OUT_OF_DEVICE_MEMORY:
		return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
	case VK_ERROR_OUT_OF_POOL_MEMORY:
		return OUT_OF_POOL_MEMORY_MPGX_RESULT;
	case VK_ERROR_DEVICE_LOST:
		return DEVICE_IS_LOST_MPGX_RESULT;
	case VK_ERROR_SURFACE_LOST_KHR:
		return SURFACE_IS_LOST_MPGX_RESULT;
	case VK_ERROR_MEMORY_MAP_FAILED:
		return FAILED_TO_MAP_MEMORY_MPGX_RESULT;
	case VK_ERROR_INVALID_SHADER_NV:
		return BAD_SHADER_CODE_MPGX_RESULT;
	case VK_ERROR_INITIALIZATION_FAILED:
		return FAILED_TO_INITIALIZE_VULKAN_MPGX_RESULT;
	case VK_ERROR_LAYER_NOT_PRESENT:
	case VK_ERROR_EXTENSION_NOT_PRESENT:
	case VK_ERROR_FEATURE_NOT_PRESENT:
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
	case VK_SUCCESS:
		return SUCCESS_MPGX_RESULT;
	}
}

inline static MpgxResult endSubmitWaitVkCommandBuffer(
	VkDevice device,
	VkQueue queue,
	VkFence fence,
	VkCommandBuffer commandBuffer)
{
	assert(device);
	assert(queue);
	assert(fence);
	assert(commandBuffer);

	VkResult vkResult = vkEndCommandBuffer(commandBuffer);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	vkResult = vkResetFences(
		device,
		1,
		&fence);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);

	vkResult = vkWaitForFences(
		device,
		1,
		&fence,
		VK_TRUE,
		UINT64_MAX);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult allocateBeginVkCommandBuffer(
	VkDevice device,
	VkCommandPool commandPool,
	VkCommandBuffer* commandBuffer)
{
	assert(device);
	assert(commandPool);
	assert(commandBuffer);

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
		return vkToMpgxResult(vkResult);

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
		return vkToMpgxResult(vkResult);
	}

	*commandBuffer = commandBufferInstance;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult submitVkCommandBuffer(
	VkDevice device,
	VkQueue queue,
	VkFence fence,
	VkCommandBuffer commandBuffer)
{
	assert(device);
	assert(queue);
	assert(fence);
	assert(commandBuffer);

	if (fence)
	{
		VkResult vkResult = vkResetFences(
			device,
			1,
			&fence);

		if (vkResult != VK_SUCCESS)
			return vkToMpgxResult(vkResult);
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
		return vkToMpgxResult(vkResult);

	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult allocateVkDescriptorSets(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	uint32_t descriptorSetCount,
	VkDescriptorSet** descriptorSets)
{
	assert(device);
	assert(descriptorSetLayout);
	assert(descriptorPool);
	assert(descriptorSetCount > 0);
	assert(descriptorSets);

	VkDescriptorSetLayout* descriptorSetLayouts = malloc(
		descriptorSetCount * sizeof(VkDescriptorSetLayout));

	if (!descriptorSetLayouts)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkDescriptorSet* descriptorSetArray = malloc(
		descriptorSetCount * sizeof(VkDescriptorSet));

	if (!descriptorSetArray)
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
		free(descriptorSetArray);
		return vkToMpgxResult(vkResult);
	}

	*descriptorSets = descriptorSetArray;
	return SUCCESS_MPGX_RESULT;
}

inline static bool getVkCompareOperator(
	CompareOperator compareOperator,
	VkCompareOp* vkCompareOperator)
{
	assert(compareOperator < COMPARE_OPERATOR_COUNT);
	assert(vkCompareOperator);

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
#endif
