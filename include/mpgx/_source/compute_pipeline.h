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
#include "mpgx/_source/shader.h"

typedef struct BaseComputePipeline_T
{
	Window window;
	OnComputePipelineBind onBind;
	OnComputePipelineDestroy onDestroy;
	void* handle;
	Shader shader;
#ifndef NDEBUG
	const char* name;
#endif
} BaseComputePipeline_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkComputePipeline_T
{
	Window window;
	OnComputePipelineBind onBind;
	OnComputePipelineDestroy onDestroy;
	void* handle;
	Shader shader;
#ifndef NDEBUG
	const char* name;
#endif
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
} VkComputePipeline_T;
#endif
union ComputePipeline_T
{
	BaseComputePipeline_T base;
#if MPGX_SUPPORT_VULKAN
	VkComputePipeline_T vk;
#endif
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkComputePipelineCreateData
{
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pushConstantRanges;
} VkComputePipelineCreateData;

inline static MpgxResult createVkComputePipelineHandle(
	VkDevice device,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	Shader shader,
	VkPipeline* handle)
{
	assert(device);
	assert(cache);
	assert(layout);
	assert(shader);
	assert(handle);

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL,
		0,
		shader->vk.stage,
		shader->vk.handle,
		"main",
		NULL, // TODO: pass here shader dynamic constants
	};

	VkComputePipelineCreateInfo computePipelineCreateInfo = {
		VK_STRUCTURE_TYPE_COMPUTE_PIPELINE_CREATE_INFO,
		NULL,
		0,
		shaderStageCreateInfo,
		layout,
		NULL,
		0,
	};

	VkPipeline handleInstance;

	VkResult vkResult = vkCreateComputePipelines(
		device,
		cache,
		1,
		&computePipelineCreateInfo,
		NULL,
		&handleInstance);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*handle = handleInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkComputePipeline(
	VkDevice device,
	ComputePipeline computePipeline)
{
	assert(device);

	if (!computePipeline)
		return;

	vkDestroyPipeline(
		device,
		computePipeline->vk.vkHandle,
		NULL);
	vkDestroyPipelineLayout(
		device,
		computePipeline->vk.layout,
		NULL);
	vkDestroyPipelineCache(
		device,
		computePipeline->vk.cache,
		NULL);
	free(computePipeline);
}
inline static MpgxResult createVkComputePipeline(
	VkDevice device,
	const VkComputePipelineCreateData* createData,
	Window window,
	const char* name,
	OnComputePipelineBind onBind,
	OnComputePipelineDestroy onDestroy,
	void* handle,
	Shader shader,
	ComputePipeline* computePipeline)
{
	assert(device);
	assert(createData);
	assert(window);
	assert(onDestroy);
	assert(shader);
	assert(computePipeline);

	ComputePipeline computePipelineInstance = calloc(1,
		sizeof(ComputePipeline_T));

	if (!computePipelineInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

#ifndef NDEBUG
	computePipelineInstance->vk.name = name;
#endif
	computePipelineInstance->vk.window = window;
	computePipelineInstance->vk.onBind = onBind;
	computePipelineInstance->vk.onDestroy = onDestroy;
	computePipelineInstance->vk.handle = handle;
	computePipelineInstance->vk.shader = shader;

	VkPipelineCacheCreateInfo cacheCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_CACHE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
	};

	VkPipelineCache cache;

	VkResult vkResult = vkCreatePipelineCache(
		device,
		&cacheCreateInfo,
		NULL,
		&cache);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkComputePipeline(
			device,
			computePipelineInstance);
		return vkToMpgxResult(vkResult);
	}

	computePipelineInstance->vk.cache = cache;

	VkPipelineLayoutCreateInfo layoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		createData->setLayoutCount,
		createData->setLayouts,
		createData->pushConstantRangeCount,
		createData->pushConstantRanges,
	};

	VkPipelineLayout layout;

	vkResult = vkCreatePipelineLayout(
		device,
		&layoutCreateInfo,
		NULL,
		&layout);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkComputePipeline(
			device,
			computePipelineInstance);
		return vkToMpgxResult(vkResult);
	}

	computePipelineInstance->vk.layout = layout;

	VkPipeline vkHandle;

	MpgxResult mpgxResult = createVkComputePipelineHandle(
		device,
		cache,
		layout,
		shader,
		&vkHandle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkComputePipeline(
			device,
			computePipelineInstance);
		return mpgxResult;
	}

	computePipelineInstance->vk.vkHandle = vkHandle;

	*computePipeline = computePipelineInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void bindVkComputePipeline(
	VkCommandBuffer commandBuffer,
	ComputePipeline computePipeline)
{
	assert(commandBuffer);
	assert(computePipeline);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_COMPUTE,
		computePipeline->vk.vkHandle);

	if (computePipeline->vk.onBind)
		computePipeline->vk.onBind(computePipeline);
}

inline static void dispatchVkComputePipeline(
	VkCommandBuffer commandBuffer,
	uint32_t groupCountX,
	uint32_t groupCountY,
	uint32_t groupCountZ)
{
	assert(commandBuffer);
	assert(groupCountX > 0 || groupCountY > 0 || groupCountZ > 0);

	vkCmdDispatch(
		commandBuffer,
		groupCountX,
		groupCountY,
		groupCountZ);
}
#endif
