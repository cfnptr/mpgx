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

// TODO: separate pipeline/mesh/scene into different headers

#pragma once
#include "mpgx/_source/shader.h"

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

typedef struct BaseRayTracingMesh_T
{
	Window window;
	size_t vertexStride;
	IndexType indexType;
	Buffer vertexBuffer;
	Buffer indexBuffer;
} BaseRayTracingMesh_T;
typedef struct VkRayTracingMesh_T
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
} VkRayTracingMesh_T;
union RayTracingMesh_T
{
	BaseRayTracingMesh_T base;
	VkRayTracingMesh_T vk;
};

typedef struct BaseRayTracingScene_T
{
	Window window;
	RayTracingMesh* meshes;
	size_t meshCount;
} BaseRayTracingScene_T;
typedef struct VkRayTracingScene_T
{
	Window window;
	RayTracingMesh* meshes;
	size_t meshCount;
#if MPGX_SUPPORT_VULKAN
	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
#endif
} VkRayTracingScene_T;
union RayTracingScene_T
{
	BaseRayTracingScene_T base;
	VkRayTracingScene_T vk;
};

typedef struct BaseRayTracing_T
{
	RayTracingPipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	RayTracingMesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	RayTracingScene* scenes;
	size_t sceneCapacity;
	size_t sceneCount;
} BaseRayTracing_T;
typedef struct VkRayTracing_T
{
	RayTracingPipeline* pipelines;
	size_t pipelineCapacity;
	size_t pipelineCount;
	RayTracingMesh* meshes;
	size_t meshCapacity;
	size_t meshCount;
	RayTracingScene* scenes;
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
inline static VkPipeline createVkRayTracingPipelineHandle(
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

inline static bool createVkRayTracingBuffer(
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
inline static bool unmapVkRayTracingBuffer(
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

	bool result = createVkRayTracingBuffer(
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

	result = unmapVkRayTracingBuffer(
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

	result = createVkRayTracingBuffer(
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

	result = unmapVkRayTracingBuffer(
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

	result = createVkRayTracingBuffer(
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

	result = unmapVkRayTracingBuffer(
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

inline static void destroyVkRayTracingPipeline(
	VkDevice device,
	VmaAllocator allocator,
	RayTracingPipeline rayTracingPipeline,
	bool destroyShaders)
{
	if (rayTracingPipeline == NULL)
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

	if (destroyShaders == true)
	{
		Shader* shaders = rayTracingPipeline->vk.closestHitShaders;
		size_t shaderCount = rayTracingPipeline->vk.closestHitShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);

		shaders = rayTracingPipeline->vk.missShaders;
		shaderCount = rayTracingPipeline->vk.missShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);

		shaders = rayTracingPipeline->vk.generationShaders;
		shaderCount = rayTracingPipeline->vk.generationShaderCount;

		for (size_t i = 0; i < shaderCount; i++)
			destroyShader(shaders[i]);
	}

	free(rayTracingPipeline->vk.closestHitShaders);
	free(rayTracingPipeline->vk.missShaders);
	free(rayTracingPipeline->vk.generationShaders);
	free(rayTracingPipeline);
}
inline static RayTracingPipeline createVkRayTracingPipeline(
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
	size_t closestHitShaderCount)
{
	RayTracingPipeline rayTracingPipeline = calloc(1,
		sizeof(RayTracingPipeline_T));

	if (rayTracingPipeline == NULL)
		return NULL;

#ifndef NDEBUG
	rayTracingPipeline->vk.name = name;
#endif
	rayTracingPipeline->vk.window = window;
	rayTracingPipeline->vk.onBind = onBind;
	rayTracingPipeline->vk.onDestroy = onDestroy;
	rayTracingPipeline->vk.handle = handle;

	Shader* rayGenerationShaders = malloc(
		generationShaderCount * sizeof(Shader));

	if (rayGenerationShaders == NULL)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.generationShaders = rayGenerationShaders;
	rayTracingPipeline->vk.generationShaderCount = generationShaderCount;

	for (size_t i = 0; i < generationShaderCount; i++)
		rayGenerationShaders[i] = generationShaders[i];

	Shader* rayMissShaders = malloc(
		missShaderCount * sizeof(Shader));

	if (rayMissShaders == NULL)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.missShaders = rayMissShaders;
	rayTracingPipeline->vk.missShaderCount = missShaderCount;

	for (size_t i = 0; i < missShaderCount; i++)
		rayMissShaders[i] = missShaders[i];

	Shader* rayClosestHitShaders = malloc(
		closestHitShaderCount * sizeof(Shader));

	if (rayClosestHitShaders == NULL)
	{
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.closestHitShaders = rayClosestHitShaders;
	rayTracingPipeline->vk.closestHitShaderCount = closestHitShaderCount;

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
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.cache = cache;

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
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.layout = layout;

	VkPipeline vkHandle = createVkRayTracingPipelineHandle(
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
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.vkHandle = vkHandle;

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
		destroyVkRayTracingPipeline(
			device,
			allocator,
			rayTracingPipeline,
			false);
		return NULL;
	}

	rayTracingPipeline->vk.generationSbtBuffer = generationSbtBuffer;
	rayTracingPipeline->vk.generationSbtAllocation = generationSbtAllocation;
	rayTracingPipeline->vk.generationSbtAddress = generationSbtAddress;
	rayTracingPipeline->vk.missSbtBuffer = missSbtBuffer;
	rayTracingPipeline->vk.missSbtAllocation = missSbtAllocation;
	rayTracingPipeline->vk.missSbtAddress = missSbtAddress;
	rayTracingPipeline->vk.closestHitSbtBuffer = closestHitSbtBuffer;
	rayTracingPipeline->vk.closestHitSbtAllocation = closestHitSbtAllocation;
	rayTracingPipeline->vk.closestHitSbtAddress = closestHitSbtAddress;
	return rayTracingPipeline;
}

inline static void bindVkRayTracingPipeline(
	VkCommandBuffer commandBuffer,
	RayTracingPipeline rayTracingPipeline)
{
	vkCmdBindPipeline(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		rayTracingPipeline->vk.vkHandle);

	if (rayTracingPipeline->vk.onBind != NULL)
		rayTracingPipeline->vk.onBind(rayTracingPipeline);
}
inline static void traceVkPipelineRays(
	VkCommandBuffer commandBuffer,
	RayTracing rayTracing,
	RayTracingPipeline rayTracingPipeline)
{
	uint32_t handleSize = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleSize;
	uint32_t handleAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupHandleAlignment;
	uint32_t baseAlignment = rayTracing->vk.
		rayTracingPipelineProperties.shaderGroupBaseAlignment;
	uint32_t handleSizeAligned = alignVkSbt(handleSize, handleAlignment);

	VkDeviceSize size = alignVkSbt(
		rayTracingPipeline->vk.generationShaderCount * handleSizeAligned,
		baseAlignment);
	VkStridedDeviceAddressRegionKHR generationSbt = {
		rayTracingPipeline->vk.generationSbtAddress,
		size,
		size,
	};

	VkStridedDeviceAddressRegionKHR missSbt = {
		rayTracingPipeline->vk.missSbtAddress,
		alignVkSbt(rayTracingPipeline->vk.missShaderCount *
			handleSizeAligned, baseAlignment),
		handleSizeAligned,
	};
	VkStridedDeviceAddressRegionKHR closestHitSbt = {
		rayTracingPipeline->vk.closestHitSbtAddress,
		alignVkSbt(rayTracingPipeline->vk.closestHitShaderCount *
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

	bool result = createVkRayTracingBuffer(
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

	result = createVkRayTracingBuffer(
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

inline static void destroyVkRayTracingMesh(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayTracingMesh rayTracingMesh,
	bool destroyBuffers)
{
	if (rayTracingMesh == NULL)
		return;

	rayTracing->vk.destroyAccelerationStructure(
		device,
		rayTracingMesh->vk.accelerationStructure,
		NULL);
	vmaDestroyBuffer(
		allocator,
		rayTracingMesh->vk.buffer,
		rayTracingMesh->vk.allocation);

	if (destroyBuffers == true)
	{
		destroyBuffer(rayTracingMesh->vk.indexBuffer);
		destroyBuffer(rayTracingMesh->vk.vertexBuffer);
	}

	free(rayTracingMesh);
}
inline static RayTracingMesh createVkRayTracingMesh(
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
	RayTracingMesh rayTracingMesh = calloc(1,
		sizeof(RayTracingMesh_T));

	if (rayTracingMesh == NULL)
		return NULL;

	rayTracingMesh->vk.window = window;

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
		destroyVkRayTracingMesh(
			device,
			allocator,
			rayTracing,
			rayTracingMesh,
			false);
		return NULL;
	}

	bufferDeviceAddressInfo.buffer = indexBuffer->vk.handle;

	VkDeviceAddress indexBufferAddress = vkGetBufferDeviceAddress(
		device, &bufferDeviceAddressInfo);

	if (indexBufferAddress == 0)
	{
		destroyVkRayTracingMesh(
			device,
			allocator,
			rayTracing,
			rayTracingMesh,
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
		destroyVkRayTracingMesh(
			device,
			allocator,
			rayTracing,
			rayTracingMesh,
			false);
		return NULL;
	}

	rayTracingMesh->vk.buffer = buffer;
	rayTracingMesh->vk.allocation = allocation;
	rayTracingMesh->vk.accelerationStructure = accelerationStructure;
	rayTracingMesh->vk.deviceAddress = deviceAddress;
	return rayTracingMesh;
}

inline static void destroyVkRayTracingScene(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayTracingScene rayTracingScene)
{
	if (rayTracingScene == NULL)
		return;

	rayTracing->vk.destroyAccelerationStructure(
		device,
		rayTracingScene->vk.accelerationStructure,
		NULL);
	vmaDestroyBuffer(
		allocator,
		rayTracingScene->vk.buffer,
		rayTracingScene->vk.allocation);
	free(rayTracingScene->vk.meshes);
	free(rayTracingScene);
}
inline static RayTracingScene createVkRayTracingScene(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandPool transferCommandPool,
	VkFence transferFence,
	RayTracing rayTracing,
	Window window,
	RayTracingMesh* meshes,
	size_t meshCount)
{
	RayTracingScene rayTracingScene = calloc(1,
		sizeof(RayTracingScene_T));

	if (rayTracingScene == NULL)
		return NULL;

	rayTracingScene->vk.window = window;

	RayTracingMesh* rayTracingMeshes = malloc(
		meshCount * sizeof(RayTracingMesh));

	if (meshes == NULL)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingScene);
		return NULL;
	}

	rayTracingScene->vk.meshes = rayTracingMeshes;
	rayTracingScene->vk.meshCount = meshCount;

	for (size_t i = 0; i < meshCount; i++)
		rayTracingMeshes[i] = meshes[i];

	size_t instanceBufferSize = meshCount *
		sizeof(VkAccelerationStructureInstanceKHR);
	VkAccelerationStructureInstanceKHR* instances =
		malloc(instanceBufferSize);

	if (instances == NULL)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingScene);
		return NULL;
	}

	VkTransformMatrixKHR transformMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f
	};

	for (size_t i = 0; i < meshCount; i++)
	{
		VkAccelerationStructureInstanceKHR instance = {
			transformMatrix,
			(uint32_t)i,
			0xFF, // TODO: set mask from argument
			0,
			VK_GEOMETRY_INSTANCE_TRIANGLE_FACING_CULL_DISABLE_BIT_KHR,
			meshes[i]->vk.deviceAddress,
		};

		instances[i] = instance;
	}

	VkBuffer instanceBuffer;
	VmaAllocation instanceAllocation;
	VkDeviceAddress instanceBufferAddress;

	bool result = createVkRayTracingBuffer(
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
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingScene);
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

	uint32_t primitiveCount = (uint32_t)meshCount;

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
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingScene);
		return NULL;
	}

	rayTracingScene->vk.buffer = buffer;
	rayTracingScene->vk.allocation = allocation;
	rayTracingScene->vk.accelerationStructure = accelerationStructure;
	return rayTracingScene;
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

	RayTracingPipeline* pipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingPipeline));

	if (pipelines == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.pipelines = pipelines;
	rayTracing->vk.pipelineCount = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.pipelineCapacity = 0;

	RayTracingMesh* meshes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingMesh));

	if (meshes == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.meshes = meshes;
	rayTracing->vk.meshCount = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.meshCapacity = 0;

	RayTracingScene* scenes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingScene));

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
