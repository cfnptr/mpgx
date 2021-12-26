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
#include "mpgx/_source/shader.h"

typedef struct BaseRayPipeline_T
{
	Window window;
	OnRayPipelineBind onBind;
	OnRayPipelineResize onResize;
	OnRayPipelineDestroy onDestroy;
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
} BaseRayPipeline_T;
typedef struct VkRayPipeline_T
{
	Window window;
	OnRayPipelineBind onBind;
	OnRayPipelineResize onResize;
	OnRayPipelineDestroy onDestroy;
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
#if MPGX_SUPPORT_VULKAN
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
#endif
} VkRayPipeline_T;
union RayPipeline_T
{
	BaseRayPipeline_T base;
	VkRayPipeline_T vk;
};

#if MPGX_SUPPORT_VULKAN
typedef struct VkRayPipelineCreateInfo
{
	uint32_t setLayoutCount;
	const VkDescriptorSetLayout* setLayouts;
	uint32_t pushConstantRangeCount;
	const VkPushConstantRange* pushConstantRanges;
} VkRayPipelineCreateInfo;
#endif

typedef struct BaseRayMesh_T
{
	Window window;
	size_t vertexStride;
	IndexType indexType;
	Buffer vertexBuffer;
	Buffer indexBuffer;
} BaseRayMesh_T;
typedef struct VkRayMesh_T
{
	Window window;
	size_t vertexStride;
	IndexType indexType;
	Buffer vertexBuffer;
	Buffer indexBuffer;
#if MPGX_SUPPORT_VULKAN
	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
	VkDeviceAddress deviceAddress;
#endif
} VkRayMesh_T;
union RayMesh_T
{
	BaseRayMesh_T base;
	VkRayMesh_T vk;
};

typedef struct BaseRayScene_T
{
	Window window;
	RayMesh* meshes;
	size_t meshCount;
} BaseRayScene_T;
typedef struct VkRayScene_T
{
	Window window;
	RayMesh* meshes;
	size_t meshCount;
#if MPGX_SUPPORT_VULKAN
	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
#endif
} VkRayScene_T;
union RayScene_T
{
	BaseRayScene_T base;
	VkRayScene_T vk;
};

typedef struct BaseRayTracing_T
{
	RayPipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	RayMesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	RayScene* scenes;
	size_t sceneCapacity;
	size_t sceneCount;
} BaseRayTracing_T;
typedef struct VkRayTracing_T
{
	RayPipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	RayMesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	RayScene* scenes;
	size_t sceneCapacity;
	size_t sceneCount;
#if MPGX_SUPPORT_VULKAN
	VkPhysicalDeviceRayTracingPipelinePropertiesKHR
		rayTracingPipelineProperties;
	PFN_vkGetAccelerationStructureBuildSizesKHR
		getAccelerationStructureBuildSizes;
	PFN_vkCreateAccelerationStructureKHR
		createAccelerationStructure;
	PFN_vkDestroyAccelerationStructureKHR
		destroyAccelerationStructure;
	PFN_vkCmdBuildAccelerationStructuresKHR
		cmdBuildAccelerationStructures;
	PFN_vkGetAccelerationStructureDeviceAddressKHR
		getAccelerationStructureDeviceAddress;
	PFN_vkCreateRayTracingPipelinesKHR
		createRayTracingPipelinesKHR;
	PFN_vkGetRayTracingShaderGroupHandlesKHR
		getRayTracingShaderGroupHandles;
	PFN_vkCmdTraceRaysKHR cmdTraceRays;
#endif
} VkRayTracing_T;
typedef union RayTracing_T
{
	BaseRayTracing_T base;
	VkRayTracing_T vk;
} RayTracing_T;

typedef RayTracing_T* RayTracing;

#if MPGX_SUPPORT_VULKAN
inline static VkPipeline createVkRayPipelineHandle(
	VkDevice device,
	VkPipelineCache cache,
	VkPipelineLayout layout,
	RayTracing rayTracing,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount)
{
	size_t shaderCount =
		generationShaderCount +
		missShaderCount +
		closestHitShaderCount;

	VkPipelineShaderStageCreateInfo* shaderStageCreateInfos =
		malloc(shaderCount * sizeof(VkPipelineShaderStageCreateInfo));

	if (shaderStageCreateInfos == NULL)
		return NULL;

	VkRayTracingShaderGroupCreateInfoKHR* shaderGroupCreateInfos =
		malloc(shaderCount * sizeof(VkRayTracingShaderGroupCreateInfoKHR));

	if (shaderGroupCreateInfos == NULL)
	{
		free(shaderStageCreateInfos);
		return NULL;
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

	VkPipeline handle;

	VkResult vkResult = rayTracing->vk.createRayTracingPipelinesKHR(
		device,
		NULL,
		cache,
		1,
		&rayTracingPipelineCreateInfo,
		NULL,
		&handle);

	free(shaderGroupCreateInfos);
	free(shaderStageCreateInfos);

	if (vkResult != VK_SUCCESS)
		return NULL;

	return handle;
}

inline static bool createVkRayBuffer(
	VkDevice device,
	VmaAllocator allocator,
	size_t size,
	VkBufferUsageFlags usage,
	VkBuffer* buffer,
	VmaAllocation* allocation,
	VkDeviceAddress* address,
	void* data,
	uint8_t** mappedData)
{
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

	VmaAllocationCreateInfo allocationCreateInfo;
	memset(&allocationCreateInfo, 0, sizeof(VmaAllocationCreateInfo));

	allocationCreateInfo.flags = VMA_ALLOCATION_CREATE_WITHIN_BUDGET_BIT;

	if (data != NULL || mappedData != NULL)
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;
	else
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

	VkBuffer bufferInstance;
	VmaAllocation allocationInstance;

	VkResult vkResult = vmaCreateBuffer(
		allocator,
		&bufferCreateInfo,
		&allocationCreateInfo,
		&bufferInstance,
		&allocationInstance,
		NULL);

	if (vkResult != VK_SUCCESS)
		return false;

	uint8_t* mappedBuffer;

	if (data != NULL)
	{
		void* bufferMappedData;

		vkResult = vmaMapMemory(
			allocator,
			allocationInstance,
			&bufferMappedData);

		if (vkResult != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			return false;
		}

		uint8_t* bufferMappedArray = bufferMappedData;
		memcpy(bufferMappedArray, data, size);

		if (mappedData != NULL)
		{
			mappedBuffer = bufferMappedArray;
		}
		else
		{
			vkResult = vmaFlushAllocation(
				allocator,
				allocationInstance,
				0,
				size);

			vmaUnmapMemory(
				allocator,
				allocationInstance);

			if (vkResult != VK_SUCCESS)
			{
				vmaDestroyBuffer(
					allocator,
					bufferInstance,
					allocationInstance);
				return NULL;
			}
		}
	}
	else
	{
		if (mappedData != NULL)
		{
			void* bufferMappedData;

			vkResult = vmaMapMemory(
				allocator,
				allocationInstance,
				&bufferMappedData);

			if (vkResult != VK_SUCCESS)
			{
				vmaDestroyBuffer(
					allocator,
					bufferInstance,
					allocationInstance);
				return false;
			}

			mappedBuffer = bufferMappedData;
		}
	}

	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		bufferInstance,
	};

	VkDeviceAddress bufferAddress;

	if (address != NULL)
	{
		bufferAddress = vkGetBufferDeviceAddress(
			device, &bufferDeviceAddressInfo);

		if (bufferAddress == 0)
		{
			if (mappedData != NULL)
			{
				vmaUnmapMemory(
					allocator,
					allocationInstance);
			}
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			return false;
		}
	}

	*buffer = bufferInstance;
	*allocation = allocationInstance;

	if (mappedData != NULL)
		*mappedData = mappedBuffer;
	if (address != NULL)
		*address = bufferAddress;
	return true;
}
inline static bool unmapVkRayBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	size_t size)
{
	VkResult vkResult = vmaFlushAllocation(
		allocator,
		allocation,
		0,
		size);

	vmaUnmapMemory(
		allocator,
		allocation);

	if (vkResult != VK_SUCCESS)
		return false;

	return true;
}

inline static uint32_t alignVkSbt(
	uint32_t handleSize,
	uint32_t handleAlignment)
{
	return (handleSize + (handleAlignment - 1)) & ~(handleAlignment - 1);
}
inline static bool createVkSbt(
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
	uint32_t handleSize = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = alignVkSbt(handleSize, handleAlignment);

	uint32_t handleCount =
		generationShaderCount +
		missShaderCount +
		closestHitShaderCount;

	uint32_t shaderGroupHandleSize = handleCount * handleSize;
	uint8_t* shaderGroupHandles = malloc(shaderGroupHandleSize);

	if (shaderGroupHandles == NULL)
		return false;

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
		return NULL;
	}

	size_t bufferSize = alignVkSbt(
		generationShaderCount * handleSizeAligned,
		baseAlignment);
	uint8_t* mappedData;

	VkBuffer rayGenerationSbtBuffer;
	VmaAllocation rayGenerationSbtAllocation;
	VkDeviceAddress rayGenerationSbtAddress;

	bool result = createVkRayBuffer(
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

	if (result == false)
	{
		free(shaderGroupHandles);
		return NULL;
	}

	size_t shaderHandleIndex = 0;

	for (size_t i = 0; i < generationShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	result = unmapVkRayBuffer(
		allocator,
		rayGenerationSbtAllocation,
		bufferSize);

	if (result == false)
	{
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return NULL;
	}

	bufferSize = alignVkSbt(
		missShaderCount * handleSizeAligned,
		baseAlignment);

	VkBuffer rayMissSbtBuffer;
	VmaAllocation rayMissSbtAllocation;
	VkDeviceAddress rayMissSbtAddress;

	result = createVkRayBuffer(
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

	if (result == false)
	{
		vmaDestroyBuffer(
			allocator,
			rayGenerationSbtBuffer,
			rayGenerationSbtAllocation);
		free(shaderGroupHandles);
		return NULL;
	}

	for (size_t i = 0; i < missShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	result = unmapVkRayBuffer(
		allocator,
		rayMissSbtAllocation,
		bufferSize);

	if (result == false)
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
		return NULL;
	}

	bufferSize = alignVkSbt(
		closestHitShaderCount * handleSizeAligned,
		baseAlignment);

	VkBuffer rayClosestHitSbtBuffer;
	VmaAllocation rayClosestHitSbtAllocation;
	VkDeviceAddress rayClosestHitSbtAddress;

	result = createVkRayBuffer(
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

	if (result == false)
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
		return NULL;
	}

	for (size_t i = 0; i < closestHitShaderCount; i++)
	{
		memcpy(mappedData + (i * handleSizeAligned),
			shaderGroupHandles + (shaderHandleIndex * handleSize),
			handleSize);
		shaderHandleIndex++;
	}

	result = unmapVkRayBuffer(
		allocator,
		rayClosestHitSbtAllocation,
		bufferSize);

	if (result == false)
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
		return NULL;
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
	return true;
}

inline static void destroyVkRayPipeline(
	VkDevice device,
	VmaAllocator allocator,
	RayPipeline rayPipeline,
	bool destroyShaders)
{
	if (rayPipeline == NULL)
		return;

	vmaDestroyBuffer(
		allocator,
		rayPipeline->vk.closestHitSbtBuffer,
		rayPipeline->vk.closestHitSbtAllocation);
	vmaDestroyBuffer(
		allocator,
		rayPipeline->vk.missSbtBuffer,
		rayPipeline->vk.missSbtAllocation);
	vmaDestroyBuffer(
		allocator,
		rayPipeline->vk.generationSbtBuffer,
		rayPipeline->vk.generationSbtAllocation);
	vkDestroyPipeline(
		device,
		rayPipeline->vk.vkHandle,
		NULL);
	vkDestroyPipelineLayout(
		device,
		rayPipeline->vk.layout,
		NULL);
	vkDestroyPipelineCache(
		device,
		rayPipeline->vk.cache,
		NULL);

	if (destroyShaders == true)
	{
		Shader* shaders = rayPipeline->vk.closestHitShaders;
		size_t shaderCount = rayPipeline->vk.closestHitShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);

		shaders = rayPipeline->vk.missShaders;
		shaderCount = rayPipeline->vk.missShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);

		shaders = rayPipeline->vk.generationShaders;
		shaderCount = rayPipeline->vk.generationShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(rayPipeline->vk.closestHitShaders);
	free(rayPipeline->vk.missShaders);
	free(rayPipeline->vk.generationShaders);
	free(rayPipeline);
}
inline static RayPipeline createVkRayPipeline(
	VkDevice device,
	VmaAllocator allocator,
	const VkRayPipelineCreateInfo* createInfo,
	RayTracing rayTracing,
	const char* name,
	Window window,
	OnRayPipelineBind onBind,
	OnRayPipelineResize onResize,
	OnRayPipelineDestroy onDestroy,
	void* handle,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount)
{
	RayPipeline rayPipeline = calloc(1,
		sizeof(RayPipeline_T));

	if (rayPipeline == NULL)
		return NULL;

#ifndef NDEBUG
	rayPipeline->vk.name = name;
#endif
	rayPipeline->vk.window = window;
	rayPipeline->vk.onBind = onBind;
	rayPipeline->vk.onResize = onResize;
	rayPipeline->vk.onDestroy = onDestroy;
	rayPipeline->vk.handle = handle;

	Shader* rayGenerationShaders = malloc(
		generationShaderCount * sizeof(Shader));

	if (rayGenerationShaders == NULL)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.generationShaders = rayGenerationShaders;
	rayPipeline->vk.generationShaderCount = generationShaderCount;

	for (size_t i = 0; i < generationShaderCount; i++)
		rayGenerationShaders[i] = generationShaders[i];

	Shader* rayMissShaders = malloc(
		missShaderCount * sizeof(Shader));

	if (rayMissShaders == NULL)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.missShaders = rayMissShaders;
	rayPipeline->vk.missShaderCount = missShaderCount;

	for (size_t i = 0; i < missShaderCount; i++)
		rayMissShaders[i] = missShaders[i];

	Shader* rayClosestHitShaders = malloc(
		closestHitShaderCount * sizeof(Shader));

	if (rayClosestHitShaders == NULL)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.closestHitShaders = rayClosestHitShaders;
	rayPipeline->vk.closestHitShaderCount = closestHitShaderCount;

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
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.cache = cache;

	VkPipelineLayoutCreateInfo layoutCreateInfo = {
		VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
		NULL,
		0,
		createInfo->setLayoutCount,
		createInfo->setLayouts,
		createInfo->pushConstantRangeCount,
		createInfo->pushConstantRanges,
	};

	VkPipelineLayout layout;

	vkResult = vkCreatePipelineLayout(
		device,
		&layoutCreateInfo,
		NULL,
		&layout);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.layout = layout;

	VkPipeline vkHandle = createVkRayPipelineHandle(
		device,
		cache,
		layout,
		rayTracing,
		generationShaders,
		generationShaderCount,
		missShaders,
		missShaderCount,
		closestHitShaders,
		closestHitShaderCount);

	if (vkHandle == NULL)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.vkHandle = vkHandle;

	VkBuffer generationSbtBuffer;
	VmaAllocation generationSbtAllocation;
	VkDeviceAddress generationSbtAddress;
	VkBuffer missSbtBuffer;
	VmaAllocation missSbtAllocation;
	VkDeviceAddress missSbtAddress;
	VkBuffer closestHitSbtBuffer;
	VmaAllocation closestHitSbtAllocation;
	VkDeviceAddress closestHitSbtAddress;

	bool result = createVkSbt(
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

	if (result == false)
	{
		destroyVkRayPipeline(
			device,
			allocator,
			rayPipeline,
			false);
		return NULL;
	}

	rayPipeline->vk.generationSbtBuffer = generationSbtBuffer;
	rayPipeline->vk.generationSbtAllocation = generationSbtAllocation;
	rayPipeline->vk.generationSbtAddress = generationSbtAddress;
	rayPipeline->vk.missSbtBuffer = missSbtBuffer;
	rayPipeline->vk.missSbtAllocation = missSbtAllocation;
	rayPipeline->vk.missSbtAddress = missSbtAddress;
	rayPipeline->vk.closestHitSbtBuffer = closestHitSbtBuffer;
	rayPipeline->vk.closestHitSbtAllocation = closestHitSbtAllocation;
	rayPipeline->vk.closestHitSbtAddress = closestHitSbtAddress;
	return rayPipeline;
}

inline static void bindVkRayPipeline(
	VkCommandBuffer commandBuffer,
	RayPipeline rayPipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		rayPipeline->vk.vkHandle);

	if (rayPipeline->vk.onBind != NULL)
		rayPipeline->vk.onBind(rayPipeline);
}
inline static void traceVkPipelineRays(
	VkCommandBuffer commandBuffer,
	RayTracing rayTracing,
	RayPipeline rayPipeline)
{
	uint32_t handleSize = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = alignVkSbt(handleSize, handleAlignment);

	VkDeviceSize size = alignVkSbt(
		rayPipeline->vk.generationShaderCount * handleSizeAligned, baseAlignment);
	VkStridedDeviceAddressRegionKHR generationSbt = {
		rayPipeline->vk.generationSbtAddress,
		size,
		size,
	};

	VkStridedDeviceAddressRegionKHR missSbt = {
		rayPipeline->vk.missSbtAddress,
		alignVkSbt(rayPipeline->vk.missShaderCount *
			handleSizeAligned, baseAlignment),
		handleSizeAligned,
	};
	VkStridedDeviceAddressRegionKHR closestHitSbt = {
		rayPipeline->vk.closestHitSbtAddress,
		alignVkSbt(rayPipeline->vk.closestHitShaderCount *
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

inline static bool createBuildVkAccelerationStructure(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	RayTracing rayTracing,
	VkAccelerationStructureTypeKHR type,
	VkAccelerationStructureBuildGeometryInfoKHR* buildGeometryInfo,
	uint32_t primitiveCount,
	VkBuffer* buffer,
	VmaAllocation* allocation,
	VkAccelerationStructureKHR* accelerationStructure,
	VkDeviceAddress* deviceAddress)
{
	VkAccelerationStructureBuildSizesInfoKHR buildSizeInfo;
	buildSizeInfo.sType = VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_SIZES_INFO_KHR;
	buildSizeInfo.pNext = NULL;

	rayTracing->vk.getAccelerationStructureBuildSizes(
		device,
		VK_ACCELERATION_STRUCTURE_BUILD_TYPE_DEVICE_KHR,
		buildGeometryInfo,
		&primitiveCount,
		&buildSizeInfo);

	VkBuffer bufferInstance;
	VmaAllocation allocationInstance;

	bool result = createVkRayBuffer(
		device,
		allocator,
		buildSizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&bufferInstance,
		&allocationInstance,
		NULL,
		NULL,
		NULL);

	if (result == false)
		return false;

	VkAccelerationStructureCreateInfoKHR accelerationStructureCreateInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_CREATE_INFO_KHR,
		NULL,
		0,
		bufferInstance,
		0,
		buildSizeInfo.accelerationStructureSize,
		type,
		0,
	};

	VkAccelerationStructureKHR accelerationStructureInstance;

	VkResult vkResult = rayTracing->vk.createAccelerationStructure(
		device,
		&accelerationStructureCreateInfo,
		NULL,
		&accelerationStructureInstance);

	if (vkResult != VK_SUCCESS)
	{
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		return false;
	}

	VkBuffer scratchBuffer;
	VmaAllocation scratchAllocation;
	VkDeviceAddress scratchBufferAddress;

	result = createVkRayBuffer(
		device,
		allocator,
		buildSizeInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&scratchBuffer,
		&scratchAllocation,
		&scratchBufferAddress,
		NULL,
		NULL);

	if (result == false)
	{
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		rayTracing->vk.destroyAccelerationStructure(
			device,
			accelerationStructureInstance,
			NULL);
		return false;
	}

	buildGeometryInfo->dstAccelerationStructure = accelerationStructureInstance;
	buildGeometryInfo->scratchData.deviceAddress = scratchBufferAddress;

	VkAccelerationStructureBuildRangeInfoKHR buildRangeInfo = {
		primitiveCount,
		0,
		0,
		0,
	};
	const VkAccelerationStructureBuildRangeInfoKHR* buildRangeInfos[1] = {
		&buildRangeInfo,
	};

	VkCommandBuffer commandBuffer = allocateBeginVkOneTimeCommandBuffer(
		device, transferCommandPool);

	if (commandBuffer == NULL)
	{
		vmaDestroyBuffer(
			allocator,
			scratchBuffer,
			scratchAllocation);
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		rayTracing->vk.destroyAccelerationStructure(
			device,
			accelerationStructureInstance,
			NULL);
		return false;
	}

	rayTracing->vk.cmdBuildAccelerationStructures(
		commandBuffer,
		1,
		buildGeometryInfo,
		buildRangeInfos);

	result = endSubmitWaitFreeVkCommandBuffer(
		device,
		transferQueue,
		transferCommandPool,
		transferFence,
		commandBuffer);

	vmaDestroyBuffer(
		allocator,
		scratchBuffer,
		scratchAllocation);

	if (result == false)
	{
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		rayTracing->vk.destroyAccelerationStructure(
			device,
			accelerationStructureInstance,
			NULL);
		return false;
	}

	// TODO: compact acceleration structure

	if (deviceAddress != NULL)
	{
		VkAccelerationStructureDeviceAddressInfoKHR accelerationDeviceAddressInfo = {
			VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_DEVICE_ADDRESS_INFO_KHR,
			NULL,
			accelerationStructureInstance,
		};

		VkDeviceAddress address = rayTracing->vk.getAccelerationStructureDeviceAddress(
			device,
			&accelerationDeviceAddressInfo);

		if (address == 0)
		{
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			rayTracing->vk.destroyAccelerationStructure(
				device,
				accelerationStructureInstance,
				NULL);
			return false;
		}

		*deviceAddress = address;
	}

	*buffer = bufferInstance;
	*allocation = allocationInstance;
	*accelerationStructure = accelerationStructureInstance;
	return true;
}

inline static void destroyVkRayMesh(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayMesh rayMesh,
	bool destroyBuffers)
{
	if (rayMesh == NULL)
		return;

	rayTracing->vk.destroyAccelerationStructure(
		device,
		rayMesh->vk.accelerationStructure,
		NULL);
	vmaDestroyBuffer(
		allocator,
		rayMesh->vk.buffer,
		rayMesh->vk.allocation);

	if (destroyBuffers == true)
	{
		destroyBuffer(rayMesh->vk.indexBuffer);
		destroyBuffer(rayMesh->vk.vertexBuffer);
	}

	free(rayMesh);
}
inline static RayMesh createVkRayMesh(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	RayTracing rayTracing,
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer)
{
	RayMesh rayMesh = calloc(1,
		sizeof(RayMesh_T));

	if (rayMesh == NULL)
		return NULL;

	rayMesh->vk.window = window;

	VkIndexType vkIndexType;
	size_t indexSize;

	if (indexType == UINT16_INDEX_TYPE)
	{
		vkIndexType = VK_INDEX_TYPE_UINT16;
		indexSize = sizeof(uint16_t);
	}
	else if (indexType == UINT32_INDEX_TYPE)
	{
		vkIndexType = VK_INDEX_TYPE_UINT32;
		indexSize = sizeof(uint32_t);
	}
	else
	{
		abort();
	}

	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		vertexBuffer->vk.handle,
	};

	VkDeviceAddress vertexBufferAddress = vkGetBufferDeviceAddress(
		device, &bufferDeviceAddressInfo);

	if (vertexBufferAddress == 0)
	{
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh,
			false);
		return NULL;
	}

	bufferDeviceAddressInfo.buffer = indexBuffer->vk.handle;

	VkDeviceAddress indexBufferAddress = vkGetBufferDeviceAddress(
		device, &bufferDeviceAddressInfo);

	if (indexBufferAddress == 0)
	{
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh,
			false);
		return NULL;
	}

	VkDeviceOrHostAddressConstKHR vertexAddress;
	vertexAddress.deviceAddress = vertexBufferAddress;
	VkDeviceOrHostAddressConstKHR indexAddress;
	indexAddress.deviceAddress = indexBufferAddress;
	VkDeviceOrHostAddressConstKHR transformAddress;
	transformAddress.deviceAddress = 0; // TODO: transform buffer

	uint32_t vertexCount = (uint32_t)(vertexBuffer->vk.size / vertexStride);

	VkAccelerationStructureGeometryTrianglesDataKHR geometryTrianglesData = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
		NULL,
		VK_FORMAT_R32G32B32_SFLOAT,
		vertexAddress,
		vertexStride,
		vertexCount,
		vkIndexType,
		indexAddress,
		transformAddress,
	};

	VkAccelerationStructureGeometryDataKHR geometryData;
	geometryData.triangles = geometryTrianglesData;

	VkAccelerationStructureGeometryKHR geometry = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		NULL,
		VK_GEOMETRY_TYPE_TRIANGLES_KHR,
		geometryData,
		VK_GEOMETRY_OPAQUE_BIT_KHR,
	};

	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		NULL,
		NULL,
		1,
		&geometry,
		NULL,
		0,
	};

	uint32_t primitiveCount = (uint32_t)((indexBuffer->vk.size / indexSize) / 3);

	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
	VkDeviceAddress deviceAddress;

	bool result = createBuildVkAccelerationStructure(
		device,
		allocator,
		transferQueue,
		transferCommandPool,
		transferFence,
		rayTracing,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		&buildGeometryInfo,
		primitiveCount,
		&buffer,
		&allocation,
		&accelerationStructure,
		&deviceAddress);

	if (result == false)
	{
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh,
			false);
		return NULL;
	}

	rayMesh->vk.buffer = buffer;
	rayMesh->vk.allocation = allocation;
	rayMesh->vk.accelerationStructure = accelerationStructure;
	rayMesh->vk.deviceAddress = deviceAddress;
	return rayMesh;
}

inline static void destroyVkRayScene(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayScene rayScene)
{
	if (rayScene == NULL)
		return;

	rayTracing->vk.destroyAccelerationStructure(
		device,
		rayScene->vk.accelerationStructure,
		NULL);
	vmaDestroyBuffer(
		allocator,
		rayScene->vk.buffer,
		rayScene->vk.allocation);
	free(rayScene->vk.meshes);
	free(rayScene);
}
inline static RayScene createVkRayScene(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	RayTracing rayTracing,
	Window window,
	RayMesh* rayMeshes,
	size_t rayMeshCount)
{
	RayScene rayScene = calloc(1,
		sizeof(RayScene_T));

	if (rayScene == NULL)
		return NULL;

	rayScene->vk.window = window;

	RayMesh* meshes = malloc(
		rayMeshCount * sizeof(RayMesh));

	if (meshes == NULL)
	{
		destroyVkRayScene(
			device,
			allocator,
			rayTracing,
			rayScene);
		return NULL;
	}

	rayScene->vk.meshes = meshes;
	rayScene->vk.meshCount = rayMeshCount;

	for (size_t i = 0; i < rayMeshCount; i++)
		meshes[i] = rayMeshes[i];

	size_t instanceBufferSize = rayMeshCount *
		sizeof(VkAccelerationStructureInstanceKHR);
	VkAccelerationStructureInstanceKHR* instances =
		malloc(instanceBufferSize);

	if (instances == NULL)
	{
		destroyVkRayScene(
			device,
			allocator,
			rayTracing,
			rayScene);
		return NULL;
	}

	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	for (size_t i = 0; i < rayMeshCount; i++)
	{
		VkAccelerationStructureInstanceKHR instance = {
			transformMatrix,
			(uint32_t)i,
			0xFF, // TODO: set mask from argument
			0,
			VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			rayMeshes[i]->vk.deviceAddress,
		};

		instances[i] = instance;
	}

	VkBuffer instanceBuffer;
	VmaAllocation instanceAllocation;
	VkDeviceAddress instanceBufferAddress;

	bool result = createVkRayBuffer(
		device,
		allocator,
		instanceBufferSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&instanceBuffer,
		&instanceAllocation,
		&instanceBufferAddress,
		instances,
		NULL);

	free(instances);

	if (result == false)
	{
		destroyVkRayScene(
			device,
			allocator,
			rayTracing,
			rayScene);
		return NULL;
	}

	VkDeviceOrHostAddressConstKHR instanceAddress;
	instanceAddress.deviceAddress = instanceBufferAddress;

	VkAccelerationStructureGeometryInstancesDataKHR geometryInstancesData = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_INSTANCES_DATA_KHR,
		NULL,
		VK_FALSE,
		instanceAddress,
	};

	VkAccelerationStructureGeometryDataKHR geometryData;
	geometryData.instances = geometryInstancesData;

	VkAccelerationStructureGeometryKHR geometry = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_KHR,
		NULL,
		VK_GEOMETRY_TYPE_INSTANCES_KHR,
		geometryData,
		VK_GEOMETRY_OPAQUE_BIT_KHR,
	};
	VkAccelerationStructureBuildGeometryInfoKHR buildGeometryInfo = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_BUILD_GEOMETRY_INFO_KHR,
		NULL,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_PREFER_FAST_TRACE_BIT_KHR,
		VK_BUILD_ACCELERATION_STRUCTURE_MODE_BUILD_KHR,
		NULL,
		NULL,
		1,
		&geometry,
		NULL,
		0,
	};

	uint32_t primitiveCount = (uint32_t)rayMeshCount;

	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;

	result = createBuildVkAccelerationStructure(
		device,
		allocator,
		transferQueue,
		transferCommandPool,
		transferFence,
		rayTracing,
		VK_ACCELERATION_STRUCTURE_TYPE_TOP_LEVEL_KHR,
		&buildGeometryInfo,
		primitiveCount,
		&buffer,
		&allocation,
		&accelerationStructure,
		NULL);

	vmaDestroyBuffer(
		allocator,
		instanceBuffer,
		instanceAllocation);

	if (result == false)
	{
		destroyVkRayScene(
			device,
			allocator,
			rayTracing,
			rayScene);
		return NULL;
	}

	rayScene->vk.buffer = buffer;
	rayScene->vk.allocation = allocation;
	rayScene->vk.accelerationStructure = accelerationStructure;
	return rayScene;
}

inline static void destroyVkRayTracing(
	RayTracing rayTracing)
{
	if (rayTracing == NULL)
		return;

	assert(rayTracing->vk.sceneCount == 0);
	assert(rayTracing->vk.meshCount == 0);
	assert(rayTracing->vk.pipelineCount == 0);

	free(rayTracing->vk.scenes);
	free(rayTracing->vk.meshes);
	free(rayTracing->vk.pipelines);
	free(rayTracing);
}
inline static RayTracing createVkRayTracing(
	VkInstance instance,
	VkPhysicalDevice physicalDevice)
{
	RayTracing rayTracing = calloc(1,
		sizeof(RayTracing_T));

	if (rayTracing == NULL)
		return NULL;

	VkPhysicalDeviceRayTracingPipelinePropertiesKHR rayTracingPipelineProperties;
	rayTracingPipelineProperties.sType =
		VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_RAY_TRACING_PIPELINE_PROPERTIES_KHR;
	rayTracingPipelineProperties.pNext = NULL;

	VkPhysicalDeviceProperties2 properties2;
	properties2.sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_PROPERTIES_2;
	properties2.pNext = &rayTracingPipelineProperties;

	vkGetPhysicalDeviceProperties2(
		physicalDevice,
		&properties2);

	rayTracing->vk.rayTracingPipelineProperties = rayTracingPipelineProperties;

	RayPipeline* pipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayPipeline));

	if (pipelines == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.pipelines = pipelines;
	rayTracing->vk.pipelineCount = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.pipelineCapacity = 0;

	RayMesh* meshes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayMesh));

	if (meshes == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.meshes = meshes;
	rayTracing->vk.meshCount = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.meshCapacity = 0;

	RayScene* scenes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayScene));

	if (scenes == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.scenes = scenes;
	rayTracing->vk.sceneCount = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.sceneCapacity = 0;

	PFN_vkGetAccelerationStructureBuildSizesKHR
		getAccelerationStructureBuildSizes =
		(PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetInstanceProcAddr(
		instance, "vkGetAccelerationStructureBuildSizesKHR");

	if (getAccelerationStructureBuildSizes == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.getAccelerationStructureBuildSizes =
		getAccelerationStructureBuildSizes;

	PFN_vkCreateAccelerationStructureKHR
		createAccelerationStructure =
		(PFN_vkCreateAccelerationStructureKHR)vkGetInstanceProcAddr(
		instance, "vkCreateAccelerationStructureKHR");

	if (createAccelerationStructure == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.createAccelerationStructure =
		createAccelerationStructure;

	PFN_vkDestroyAccelerationStructureKHR
		destroyAccelerationStructure =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetInstanceProcAddr(
		instance, "vkDestroyAccelerationStructureKHR");

	if (destroyAccelerationStructure == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.destroyAccelerationStructure =
		destroyAccelerationStructure;

	PFN_vkCmdBuildAccelerationStructuresKHR
		cmdBuildAccelerationStructures =
		(PFN_vkCmdBuildAccelerationStructuresKHR)vkGetInstanceProcAddr(
		instance, "vkCmdBuildAccelerationStructuresKHR");

	if (cmdBuildAccelerationStructures == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.cmdBuildAccelerationStructures =
		cmdBuildAccelerationStructures;

	PFN_vkGetAccelerationStructureDeviceAddressKHR
		getAccelerationStructureDeviceAddress =
		(PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetInstanceProcAddr(
		instance, "vkGetAccelerationStructureDeviceAddressKHR");

	if (getAccelerationStructureDeviceAddress == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.getAccelerationStructureDeviceAddress =
		getAccelerationStructureDeviceAddress;

	PFN_vkCreateRayTracingPipelinesKHR
		createRayTracingPipelinesKHR =
		(PFN_vkCreateRayTracingPipelinesKHR)vkGetInstanceProcAddr(
		instance, "vkCreateRayTracingPipelinesKHR");

	if (createRayTracingPipelinesKHR == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.createRayTracingPipelinesKHR =
		createRayTracingPipelinesKHR;

	PFN_vkGetRayTracingShaderGroupHandlesKHR
		getRayTracingShaderGroupHandles =
		(PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetInstanceProcAddr(
		instance, "vkGetRayTracingShaderGroupHandlesKHR");

	if (getRayTracingShaderGroupHandles == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.getRayTracingShaderGroupHandles =
		getRayTracingShaderGroupHandles;

	PFN_vkCmdTraceRaysKHR cmdTraceRays =
		(PFN_vkCmdTraceRaysKHR)vkGetInstanceProcAddr(
		instance, "vkCmdTraceRaysKHR");

	if (cmdTraceRays == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.cmdTraceRays = cmdTraceRays;
	return rayTracing;
}
#endif

// TODO: cache ray tracing mesh and render:
// scratch, instance buffers
