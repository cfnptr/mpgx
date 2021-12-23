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
#include "mpgx/_source/vulkan.h"
#include "mpgx/_source/opengl.h"

typedef struct BaseRayMesh_T
{
	Window window;
} BaseRayMesh_T;
typedef struct VkRayMesh_T
{
	Window window;
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

typedef struct BaseRayRender_T
{
	Window window;
	RayMesh* rayMeshes;
	size_t rayMeshCount;
} BaseRayRender_T;
typedef struct VkRayRender_T
{
	Window window;
	RayMesh* rayMeshes;
	size_t rayMeshCount;
#if MPGX_SUPPORT_VULKAN
	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
#endif
} VkRayRender_T;
union RayRender_T
{
	BaseRayRender_T base;
	VkRayRender_T vk;
};

typedef struct BaseRayPipeline_T
{
	Window window;
} BaseRayPipeline_T;
typedef struct VkRayPipeline_T
{
	Window window;
#if MPGX_SUPPORT_VULKAN
	VkPipelineCache cache;
	VkPipeline vkHandle;
#endif
} VkRayPipeline_T;
union RayPipeline_T
{
	BaseRayPipeline_T base;
	VkRayPipeline_T vk;
};

typedef struct BaseRayTracing_T
{
	RayMesh* rayMeshes;
	size_t rayMeshCapacity;
	size_t rayMeshCount;
	RayRender* rayRenders;
	size_t rayRenderCapacity;
	size_t rayRenderCount;
} BaseRayTracing_T;
typedef struct VkRayTracing_T
{
	RayMesh* rayMeshes;
	size_t rayMeshCapacity;
	size_t rayMeshCount;
	RayRender* rayRenders;
	size_t rayRenderCapacity;
	size_t rayRenderCount;
#if MPGX_SUPPORT_VULKAN
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
#endif
} VkRayTracing_T;
typedef union RayTracing_T
{
	BaseRayTracing_T base;
	VkRayTracing_T vk;
} RayTracing_T;

typedef RayTracing_T* RayTracing;

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkRayTracing(
	RayTracing rayTracing)
{
	if (rayTracing == NULL)
		return;

	assert(rayTracing->vk.rayRenderCount == 0);
	assert(rayTracing->vk.rayMeshCount == 0);

	free(rayTracing->vk.rayRenders);
	free(rayTracing->vk.rayMeshes);
	free(rayTracing);
}
inline static RayTracing createVkRayTracing(
	VkInstance instance)
{
	RayTracing rayTracing = calloc(1,
		sizeof(RayTracing_T));

	if (rayTracing == NULL)
		return NULL;

	RayMesh* rayMeshes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayMesh));

	if (rayMeshes == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.rayMeshes = rayMeshes;
	rayTracing->vk.rayMeshCapacity = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.rayMeshCount = 0;

	RayRender* rayRenders = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayRender));

	if (rayRenders == NULL)
	{
		destroyVkRayTracing(rayTracing);
		return NULL;
	}

	rayTracing->vk.rayRenders = rayRenders;
	rayTracing->vk.rayRenderCapacity = MPGX_DEFAULT_CAPACITY;
	rayTracing->vk.rayRenderCount = 0;

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

	return rayTracing;
}

inline static bool createTmpVkBuffer(
	VkDevice device,
	VmaAllocator allocator,
	const void* data,
	size_t size,
	VkBufferUsageFlags usage,
	VkBuffer* buffer,
	VmaAllocation* allocation,
	VkDeviceAddress* address)
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

	if (data == NULL)
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
	else
		allocationCreateInfo.usage = VMA_MEMORY_USAGE_CPU_TO_GPU;

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

	if (data != NULL)
	{
		void* mappedData;

		VkResult result = vmaMapMemory(
			allocator,
			allocationInstance,
			&mappedData);

		if (result != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			return false;
		}

		memcpy(mappedData, data, size);

		result = vmaFlushAllocation(
			allocator,
			allocationInstance,
			0,
			size);

		if (result != VK_SUCCESS)
		{
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			return false;
		}

		vmaUnmapMemory(
			allocator,
			allocationInstance);
	}

	VkBufferDeviceAddressInfo bufferDeviceAddressInfo = {
		VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
		NULL,
		bufferInstance,
	};

	if (address != NULL)
	{
		VkDeviceAddress bufferAddress = vkGetBufferDeviceAddress(
			device, &bufferDeviceAddressInfo);

		if (bufferAddress == 0)
		{
			vmaDestroyBuffer(
				allocator,
				bufferInstance,
				allocationInstance);
			return false;
		}

		*address = bufferAddress;
	}

	*buffer = bufferInstance;
	*allocation = allocationInstance;
	return true;
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

	bool result = createTmpVkBuffer(
		device,
		allocator,
		NULL,
		buildSizeInfo.accelerationStructureSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_STORAGE_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&bufferInstance,
		&allocationInstance,
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

	result = createTmpVkBuffer(
		device,
		allocator,
		NULL,
		buildSizeInfo.buildScratchSize,
		VK_BUFFER_USAGE_STORAGE_BUFFER_BIT |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&scratchBuffer,
		&scratchAllocation,
		&scratchBufferAddress);

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
	RayMesh rayMesh)
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
	IndexType indexType,
	const char* vertexData,
	size_t vertexDataSize,
	const char* indexData,
	size_t indexDataSize)
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

	VkBuffer vertexBuffer;
	VmaAllocation vertexAllocation;
	VkDeviceAddress vertexBufferAddress;

	bool result = createTmpVkBuffer(
		device,
		allocator,
		vertexData,
		vertexDataSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&vertexBuffer,
		&vertexAllocation,
		&vertexBufferAddress);

	if (result == false)
	{
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh);
		return NULL;
	}

	VkBuffer indexBuffer;
	VmaAllocation indexAllocation;
	VkDeviceAddress indexBufferAddress;

	result = createTmpVkBuffer(
		device,
		allocator,
		indexData,
		indexDataSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&indexBuffer,
		&indexAllocation,
		&indexBufferAddress);

	if (result == false)
	{
		vmaDestroyBuffer(
			allocator,
			vertexBuffer,
			vertexAllocation);
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh);
		return NULL;
	}

	VkDeviceOrHostAddressConstKHR vertexAddress;
	vertexAddress.deviceAddress = vertexBufferAddress;
	VkDeviceOrHostAddressConstKHR indexAddress;
	indexAddress.deviceAddress = indexBufferAddress;
	VkDeviceOrHostAddressConstKHR transformAddress;
	transformAddress.deviceAddress = 0; // TODO: transform buffer

	uint32_t vertexCount = (uint32_t)(vertexDataSize / sizeof(Vec3F));

	// TODO: set vertex stride
	VkAccelerationStructureGeometryTrianglesDataKHR geometryTrianglesData = {
		VK_STRUCTURE_TYPE_ACCELERATION_STRUCTURE_GEOMETRY_TRIANGLES_DATA_KHR,
		NULL,
		VK_FORMAT_R32G32B32_SFLOAT,
		vertexAddress,
		sizeof(Vec3F),
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

	uint32_t primitiveCount = (uint32_t)((indexDataSize / indexSize) / 3);

	VkBuffer buffer;
	VmaAllocation allocation;
	VkAccelerationStructureKHR accelerationStructure;
	VkDeviceAddress deviceAddress;

	result = createBuildVkAccelerationStructure(
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

	vmaDestroyBuffer(
		allocator,
		indexBuffer,
		indexAllocation);
	vmaDestroyBuffer(
		allocator,
		vertexBuffer,
		vertexAllocation);

	if (result == false)
	{
		destroyVkRayMesh(
			device,
			allocator,
			rayTracing,
			rayMesh);
		return NULL;
	}

	rayMesh->vk.buffer = buffer;
	rayMesh->vk.allocation = allocation;
	rayMesh->vk.accelerationStructure = accelerationStructure;
	rayMesh->vk.deviceAddress = deviceAddress;
	return rayMesh;
}

inline static void destroyVkRayRender(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayRender rayRender)
{
	if (rayRender == NULL)
		return;

	rayTracing->vk.destroyAccelerationStructure(
		device,
		rayRender->vk.accelerationStructure,
		NULL);
	vmaDestroyBuffer(
		allocator,
		rayRender->vk.buffer,
		rayRender->vk.allocation);
	free(rayRender->vk.rayMeshes);
	free(rayRender);
}
inline static RayRender createVkRayRender(
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
	RayRender rayRender = calloc(1,
		sizeof(RayRender_T));

	if (rayRender == NULL)
		return NULL;

	rayRender->vk.window = window;

	RayMesh* renderRayMeshes = malloc(
		rayMeshCount * sizeof(RayMesh));

	if (renderRayMeshes == NULL)
	{
		destroyVkRayRender(
			device,
			allocator,
			rayTracing,
			rayRender);
		return NULL;
	}

	rayRender->vk.rayMeshes = renderRayMeshes;
	rayRender->vk.rayMeshCount = rayMeshCount;

	for (size_t i = 0; i < rayMeshCount; i++)
		renderRayMeshes[i] = rayMeshes[i];

	size_t instanceBufferSize = rayMeshCount *
		sizeof(VkAccelerationStructureInstanceKHR);
	VkAccelerationStructureInstanceKHR* instances =
		malloc(instanceBufferSize);

	if (instances == NULL)
	{
		destroyVkRayRender(
			device,
			allocator,
			rayTracing,
			rayRender);
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

	bool result = createTmpVkBuffer(
		device,
		allocator,
		instances,
		instanceBufferSize,
		VK_BUFFER_USAGE_ACCELERATION_STRUCTURE_BUILD_INPUT_READ_ONLY_BIT_KHR |
			VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
		&instanceBuffer,
		&instanceAllocation,
		&instanceBufferAddress);

	free(instances);

	if (result == false)
	{
		destroyVkRayRender(
			device,
			allocator,
			rayTracing,
			rayRender);
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
		destroyVkRayRender(
			device,
			allocator,
			rayTracing,
			rayRender);
		return NULL;
	}

	rayRender->vk.buffer = buffer;
	rayRender->vk.allocation = allocation;
	rayRender->vk.accelerationStructure = accelerationStructure;
	return rayRender;
}

inline static void destroyVkRayPipeline(
	VkDevice device,
	RayPipeline rayPipeline)
{
	if (rayPipeline == NULL)
		return;

	vkDestroyPipelineCache(
		device,
		rayPipeline->vk.cache,
		NULL);
	vkDestroyPipeline(
		device,
		rayPipeline->vk.vkHandle,
		NULL);
	free(rayPipeline);
}
inline static RayPipeline createVkRayPipeline(
	VkDevice device,
	Window window,
	Shader* shaders,
	size_t shaderCount)
{
	RayPipeline rayPipeline = calloc(1,
		sizeof(RayPipeline_T));

	if (rayPipeline == NULL)
		return NULL;

	rayPipeline->vk.window = window;

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
			rayPipeline);
		return NULL;
	}

	rayPipeline->vk.cache = cache;

	VkRayTracingPipelineCreateInfoKHR rayTracingPipelineCreateInfo = {
		VK_STRUCTURE_TYPE_RAY_TRACING_PIPELINE_CREATE_INFO_KHR,
		NULL,
		0,
		// TODO:
	};

	VkPipeline handle;

	vkResult = vkCreateRayTracingPipelinesKHR(
		device,
		VK_NULL_HANDLE,
		cache,
		1,
		&rayTracingPipelineCreateInfo,
		NULL,
		&handle);

	if (vkResult != VK_SUCCESS)
	{
		destroyVkRayPipeline(
			device,
			rayPipeline);
		return NULL;
	}

	rayPipeline->vk.vkHandle = handle;
	return rayPipeline;
}
inline static void destroyRayPipeline(
	RayPipeline rayPipeline)
{
	if (rayPipeline == NULL)
		return;
}
#endif

// TODO: cache ray tracing mesh and render:
// vertex, index, scratch, instance buffers
