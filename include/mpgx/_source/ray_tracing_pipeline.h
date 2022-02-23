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
#include "mpgx/_source/ray_tracing_scene.h"

typedef struct BaseRayTracingPipeline_T
{
	Window window;
	OnRayTracingPipelineBind onBind;
	OnRayTracingPipelineDestroy onDestroy;
	void* handle;
	Shader* generationShaders;
	size_t generationShaderCount;
	Shader* missShaders;
	size_t missShaderCount;
	Shader* closestHitShaders;
	size_t closestHitShaderCount;
	Image storageImage;
#ifndef NDEBUG
	const char* name;
#endif
} BaseRayTracingPipeline_T;
#if MPGX_SUPPORT_VULKAN
typedef struct VkRayTracingPipeline_T
{
	Window window;
	OnRayTracingPipelineBind onBind;
	OnRayTracingPipelineDestroy onDestroy;
	void* handle;
	Shader* generationShaders;
	size_t generationShaderCount;
	Shader* missShaders;
	size_t missShaderCount;
	Shader* closestHitShaders;
	size_t closestHitShaderCount;
	Image storageImage;
#ifndef NDEBUG
	const char* name;
#endif
	VkPipelineCache cache;
	VkPipelineLayout layout;
	VkPipeline vkHandle;
	VkBuffer generationSbtBuffer;
	VmaAllocation generationSbtAllocation;
	VkDeviceAddress generationSbtAddress;
	VkBuffer missSbtBuffer;
	VmaAllocation missSbtAllocation;
	VkDeviceAddress missSbtAddress;
	VkBuffer closestHitSbtBuffer;
	VmaAllocation closestHitSbtAllocation;
	VkDeviceAddress closestHitSbtAddress;
} VkRayTracingPipeline_T;
#endif
union RayTracingPipeline_T
{
	BaseRayTracingPipeline_T base;
#if MPGX_SUPPORT_VULKAN
	VkRayTracingPipeline_T vk;
#endif
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkRayTracingPipelineCreateData
{
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pushConstantRanges;
} VkRayTracingPipelineCreateData;
#endif

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult createVkRayTracingPipelineHandle(
	VkDevice device,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	RayTracing rayTracing,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount,
	VkPipeline* handle)
{
	assert(device);
	assert(cache);
	assert(layout);
	assert(rayTracing);
	assert(handle);

	// TODO: assert shader arrays

	size_t shaderCount =
		generationShaderCount +
		missShaderCount +
		closestHitShaderCount;

	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (!shaderStageCreateInfos)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkRayTracingShaderGroupCreateInfoKHR* shaderGroupCreateInfos =
		malloc(shaderCount * sizeof(VkRayTracingShaderGroupCreateInfoKHR));

	if (!shaderGroupCreateInfos)
	{
		free(shaderStageCreateInfos);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	VkPipelineShaderStageCreateInfo shaderStageCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
		NULL,
		0,
		0,
		NULL,
		"main",
		NULL, // TODO: pass here shader dynamic constants
	};
	VkRayTracingShaderGroupCreateInfoKHR shaderGroupCreateInfo = {
		VK_STRUCTURE_TYPE_RAY_TRACING_SHADER_GROUP_CREATE_INFO_KHR,
		NULL,
		VK_RAY_TRACING_SHADER_GROUP_TYPE_GENERAL_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		VK_SHADER_UNUSED_KHR,
		NULL,
	};

	size_t shaderIndex = 0;

	for (size_t i = 0; i < generationShaderCount; i++)
	{
		Shader shader = generationShaders[i];
		shaderStageCreateInfo.stage = shader->vk.stage;
		shaderStageCreateInfo.module = shader->vk.handle;
		shaderStageCreateInfos[shaderIndex] = shaderStageCreateInfo;
		shaderGroupCreateInfo.generalShader = (uint32_t)shaderIndex;
		shaderGroupCreateInfos[shaderIndex] = shaderGroupCreateInfo;
		shaderIndex++;
	}

	for (size_t i = 0; i < missShaderCount; i++)
	{
		Shader shader = missShaders[i];
		shaderStageCreateInfo.stage = shader->vk.stage;
		shaderStageCreateInfo.module = shader->vk.handle;
		shaderStageCreateInfos[shaderIndex] = shaderStageCreateInfo;
		shaderGroupCreateInfo.generalShader = (uint32_t)shaderIndex;
		shaderGroupCreateInfos[shaderIndex] = shaderGroupCreateInfo;
		shaderIndex++;
	}

	shaderGroupCreateInfo.type = VK_RAY_TRACING_SHADER_GROUP_TYPE_TRIANGLES_HIT_GROUP_KHR;
	shaderGroupCreateInfo.generalShader = VK_SHADER_UNUSED_KHR;

	for (size_t i = 0; i < closestHitShaderCount; i++)
	{
		Shader shader = closestHitShaders[i];
		shaderStageCreateInfo.stage = shader->vk.stage;
		shaderStageCreateInfo.module = shader->vk.handle;
		shaderStageCreateInfos[shaderIndex] = shaderStageCreateInfo;
		shaderGroupCreateInfo.closestHitShader = (uint32_t)shaderIndex;
		shaderGroupCreateInfos[shaderIndex] = shaderGroupCreateInfo;
		shaderIndex++;
	}

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		NULL,
		0,
		(uint32_t)shaderCount,
		shaderStageCreateInfos,
		(uint32_t)shaderCount,
		shaderGroupCreateInfos,
		1,
		NULL,
		NULL,
		NULL,
		layout,
		NULL,
		0,
	};

	VkPipeline handleInstance;

	VkResult vkResult = rayTracing->vk.createRayTracingPipelinesKHR(
		device,
		NULL,
		cache,
		1,
		&rayTracingPipelineCreateInfo,
		NULL,
		&handleInstance);

	free(shaderGroupCreateInfos);
	free(shaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
		return vkToMpgxResult(vkResult);

	*handle = handleInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static MpgxResult createVkSbt(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	VkPipeline pipeline,
	size_t generationShaderCount,
	size_t missShaderCount,
	size_t closestHitShaderCount,
	VkBuffer* generationSbtBuffer,
	VmaAllocation* generationSbtAllocation,
	VkDeviceAddress* generationSbtAddress,
	VkBuffer* missSbtBuffer,
	VmaAllocation* missSbtAllocation,
	VkDeviceAddress* missSbtAddress,
	VkBuffer* closestHitSbtBuffer,
	VmaAllocation* closestHitSbtAllocation,
	VkDeviceAddress* closestHitSbtAddress)
{
	assert(device);
	assert(allocator);
	assert(rayTracing );
	assert(pipeline);
	assert(generationSbtBuffer);
	assert(generationSbtAllocation);
	assert(generationSbtAddress);

	// TODO: assert shaders

	uint32_t handleSize = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = alignVkMemory(handleSize, handleAlignment);

	uint32_t handleCount = (uint32_t)(
		generationShaderCount +
		missShaderCount +
		closestHitShaderCount);

	uint32_t shaderGroupHandleSize = handleCount * handleSize;
	uint8_t* shaderGroupHandles = malloc(shaderGroupHandleSize);

	if (!shaderGroupHandles)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	VkResult vkResult = rayTracing->vk.getRayTracingShaderGroupHandles(
		device,
		pipeline,
		0,
		handleCount,
		shaderGroupHandleSize,
		shaderGroupHandles);

	if (vkResult != VK_SUCCESS)
	{
		free(shaderGroupHandles);
		return vkToMpgxResult(vkResult);
	}

	size_t bufferSize = alignVkMemory((uint32_t)
		generationShaderCount * handleSizeAligned,
		baseAlignment);
	uint8_t* mappedData;

	VkBuffer rayGenerationSbtBuffer;
	VmaAllocation rayGenerationSbtAllocation;
	VkDeviceAddress rayGenerationSbtAddress;

	MpgxResult mpgxResult = createVkRayTracingBuffer(
		device,
		allocator,
		bufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		&rayGenerationSbtBuffer,
		&rayGenerationSbtAllocation,
		&rayGenerationSbtAddress,
		NULL,
		&mappedData);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		free(shaderGroupHandles);
		return mpgxResult;
	}

	size_t shaderHandleIndex = 0;

	for (size_t i = 0; i < generationShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	mpgxResult = unmapVkRayTracingBuffer(
		allocator,
		rayGenerationSbtAllocation,
		bufferSize);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return mpgxResult;
	}

	bufferSize = alignVkMemory((uint32_t)
		missShaderCount * handleSizeAligned,
		baseAlignment);

	VkBuffer rayMissSbtBuffer;
	VmaAllocation rayMissSbtAllocation;
	VkDeviceAddress rayMissSbtAddress;

	mpgxResult = createVkRayTracingBuffer(
		device,
		allocator,
		bufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		&rayMissSbtBuffer,
		&rayMissSbtAllocation,
		&rayMissSbtAddress,
		NULL,
		&mappedData);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return mpgxResult;
	}

	for (size_t i = 0; i < missShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	mpgxResult = unmapVkRayTracingBuffer(
		allocator,
		rayMissSbtAllocation,
		bufferSize);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			rayMissSbtBuffer,
			rayMissSbtAllocation);
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return mpgxResult;
	}

	bufferSize = alignVkMemory((uint32_t)
		closestHitShaderCount * handleSizeAligned,
		baseAlignment);

	VkBuffer rayClosestHitSbtBuffer;
	VmaAllocation rayClosestHitSbtAllocation;
	VkDeviceAddress rayClosestHitSbtAddress;

	mpgxResult = createVkRayTracingBuffer(
		device,
		allocator,
		bufferSize,
		VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT |
		VK_BUFFER_USAGE_SHADER_BINDING_TABLE_BIT_KHR,
		&rayClosestHitSbtBuffer,
		&rayClosestHitSbtAllocation,
		&rayClosestHitSbtAddress,
		NULL,
		&mappedData);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			rayMissSbtBuffer,
			rayMissSbtAllocation);
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return mpgxResult;
	}

	for (size_t i = 0; i < closestHitShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	mpgxResult = unmapVkRayTracingBuffer(
		allocator,
		rayClosestHitSbtAllocation,
		bufferSize);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			rayClosestHitSbtBuffer,
			rayClosestHitSbtAllocation);
		vmaDestroyBuffer(
			allocator,
			rayMissSbtBuffer,
			rayMissSbtAllocation);
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return mpgxResult;
	}

	free(shaderGroupHandles);

	*generationSbtBuffer = rayGenerationSbtBuffer;
	*generationSbtAllocation = rayGenerationSbtAllocation;
	*generationSbtAddress = rayGenerationSbtAddress;
	*missSbtBuffer = rayMissSbtBuffer;
	*missSbtAllocation = rayMissSbtAllocation;
	*missSbtAddress = rayMissSbtAddress;
	*closestHitSbtBuffer = rayClosestHitSbtBuffer;
	*closestHitSbtAllocation = rayClosestHitSbtAllocation;
	*closestHitSbtAddress = rayClosestHitSbtAddress;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkRayTracingPipeline(
	VkDevice device,
	VmaAllocator allocator,
	RayTracingPipeline rayTracingPipeline)
{
	assert(device);
	assert(allocator);

	if (!rayTracingPipeline)
		return;

	vmaDestroyBuffer(
		allocator,
		rayTracingPipeline->vk.closestHitSbtBuffer,
		rayTracingPipeline->vk.closestHitSbtAllocation);
	vmaDestroyBuffer(
		allocator,
		rayTracingPipeline->vk.missSbtBuffer,
		rayTracingPipeline->vk.missSbtAllocation);
	vmaDestroyBuffer(
		allocator,
		rayTracingPipeline->vk.generationSbtBuffer,
		rayTracingPipeline->vk.generationSbtAllocation);
	vkDestroyPipeline(
		device,
		rayTracingPipeline->vk.vkHandle,
		NULL);
	vkDestroyPipelineLayout(
		device,
		rayTracingPipeline->vk.layout,
		NULL);
	vkDestroyPipelineCache(
		device,
		rayTracingPipeline->vk.cache,
		NULL);
	free(rayTracingPipeline->vk.closestHitShaders);
	free(rayTracingPipeline->vk.missShaders);
	free(rayTracingPipeline->vk.generationShaders);
	free(rayTracingPipeline);
}
inline static MpgxResult createVkRayTracingPipeline(
	VkDevice device,
	VmaAllocator allocator,
	const VkRayTracingPipelineCreateData* createData,
	RayTracing rayTracing,
	const char* name,
	Window window,
	OnRayTracingPipelineBind onBind,
	OnRayTracingPipelineDestroy onDestroy,
	void* handle,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount,
	RayTracingPipeline* rayTracingPipeline)
{
	assert(device);
	assert(allocator);
	assert(createData);
	assert(rayTracing);
	assert(window);
	assert(onDestroy);
	assert(rayTracingPipeline);

	// TODO: assert shaders

	RayTracingPipeline rayTracingPipelineInstance = calloc(1,
		sizeof(RayTracingPipeline_T));

	if (!rayTracingPipelineInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

#ifndef NDEBUG
	rayTracingPipelineInstance->vk.name = name;
#endif
	rayTracingPipelineInstance->vk.window = window;
	rayTracingPipelineInstance->vk.onBind = onBind;
	rayTracingPipelineInstance->vk.onDestroy = onDestroy;
	rayTracingPipelineInstance->vk.handle = handle;

	Shader* rayGenerationShaders = malloc(
		generationShaderCount * sizeof(Shader));

	if (!rayGenerationShaders)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingPipelineInstance->vk.generationShaders = rayGenerationShaders;
	rayTracingPipelineInstance->vk.generationShaderCount = generationShaderCount;

	for (size_t i = 0; i < generationShaderCount; i++)
		rayGenerationShaders[i] = generationShaders[i];

	Shader* rayMissShaders = malloc(
		missShaderCount * sizeof(Shader));

	if (!rayMissShaders)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingPipelineInstance->vk.missShaders = rayMissShaders;
	rayTracingPipelineInstance->vk.missShaderCount = missShaderCount;

	for (size_t i = 0; i < missShaderCount; i++)
		rayMissShaders[i] = missShaders[i];

	Shader* rayClosestHitShaders = malloc(
		closestHitShaderCount * sizeof(Shader));

	if (!rayClosestHitShaders)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingPipelineInstance->vk.closestHitShaders = rayClosestHitShaders;
	rayTracingPipelineInstance->vk.closestHitShaderCount = closestHitShaderCount;

	for (size_t i = 0; i < closestHitShaderCount; i++)
		rayClosestHitShaders[i] = closestHitShaders[i];

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
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return vkToMpgxResult(vkResult);
	}

	rayTracingPipelineInstance->vk.cache = cache;

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
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return vkToMpgxResult(vkResult);
	}

	rayTracingPipelineInstance->vk.layout = layout;

	VkPipeline vkHandle;

	MpgxResult mpgxResult = createVkRayTracingPipelineHandle(
		device,
		cache,
		layout,
		rayTracing,
		generationShaders,
		generationShaderCount,
		missShaders,
		missShaderCount,
		closestHitShaders,
		closestHitShaderCount,
		&vkHandle);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return mpgxResult;
	}

	rayTracingPipelineInstance->vk.vkHandle = vkHandle;

	VkBuffer generationSbtBuffer;
	VmaAllocation generationSbtAllocation;
	VkDeviceAddress generationSbtAddress;
	VkBuffer missSbtBuffer;
	VmaAllocation missSbtAllocation;
	VkDeviceAddress missSbtAddress;
	VkBuffer closestHitSbtBuffer;
	VmaAllocation closestHitSbtAllocation;
	VkDeviceAddress closestHitSbtAddress;

	mpgxResult = createVkSbt(
		device,
		allocator,
		rayTracing,
		vkHandle,
		generationShaderCount,
		missShaderCount,
		closestHitShaderCount,
		&generationSbtBuffer,
		&generationSbtAllocation,
		&generationSbtAddress,
		&missSbtBuffer,
		&missSbtAllocation,
		&missSbtAddress,
		&closestHitSbtBuffer,
		&closestHitSbtAllocation,
		&closestHitSbtAddress);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipelineInstance);
		return mpgxResult;
	}

	rayTracingPipelineInstance->vk.generationSbtBuffer = generationSbtBuffer;
	rayTracingPipelineInstance->vk.generationSbtAllocation = generationSbtAllocation;
	rayTracingPipelineInstance->vk.generationSbtAddress = generationSbtAddress;
	rayTracingPipelineInstance->vk.missSbtBuffer = missSbtBuffer;
	rayTracingPipelineInstance->vk.missSbtAllocation = missSbtAllocation;
	rayTracingPipelineInstance->vk.missSbtAddress = missSbtAddress;
	rayTracingPipelineInstance->vk.closestHitSbtBuffer = closestHitSbtBuffer;
	rayTracingPipelineInstance->vk.closestHitSbtAllocation = closestHitSbtAllocation;
	rayTracingPipelineInstance->vk.closestHitSbtAddress = closestHitSbtAddress;

	*rayTracingPipeline = rayTracingPipelineInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void bindVkRayTracingPipeline(
	VkCommandBuffer commandBuffer,
	RayTracingPipeline rayTracingPipeline)
{
	assert(commandBuffer);
	assert(rayTracingPipeline);

	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		rayTracingPipeline->vk.vkHandle);

	if (rayTracingPipeline->vk.onBind)
		rayTracingPipeline->vk.onBind(rayTracingPipeline);
}
inline static void traceVkPipelineRays(
	VkCommandBuffer commandBuffer,
	RayTracing rayTracing,
	RayTracingPipeline rayTracingPipeline)
{
	assert(commandBuffer);
	assert(rayTracing);
	assert(rayTracingPipeline);

	uint32_t handleSize = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = alignVkMemory(handleSize, handleAlignment);

	VkDeviceSize size = alignVkMemory((uint32_t)
		rayTracingPipeline->vk.generationShaderCount * handleSizeAligned,
		baseAlignment);
	VkStridedDeviceAddressRegionKHR generationSbt = {
		rayTracingPipeline->vk.generationSbtAddress,
		size,
		size,
	};

	VkStridedDeviceAddressRegionKHR missSbt = {
		rayTracingPipeline->vk.missSbtAddress,
		alignVkMemory((uint32_t)
			rayTracingPipeline->vk.missShaderCount *
			handleSizeAligned, baseAlignment),
		handleSizeAligned,
	};
	VkStridedDeviceAddressRegionKHR closestHitSbt = {
		rayTracingPipeline->vk.closestHitSbtAddress,
		alignVkMemory((uint32_t)
			rayTracingPipeline->vk.closestHitShaderCount *
			handleSizeAligned, baseAlignment),
		handleSizeAligned,
	};
	VkStridedDeviceAddressRegionKHR callableSbt = {
		0,
		0,
		0
	};

	rayTracing->vk.cmdTraceRays(
		commandBuffer,
		&generationSbt,
		&missSbt,
		&closestHitSbt,
		&callableSbt,
		1280, // TODO:
		720,
		1);
}
#endif
