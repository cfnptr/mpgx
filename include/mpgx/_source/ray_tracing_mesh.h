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
#include "mpgx/_source/buffer.h"
#include "mpgx/_source/ray_tracing.h"

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

#if MPGX_SUPPORT_VULKAN
inline static MpgxResult createVkRayTracingBuffer(
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
	assert(device != NULL);
	assert(allocator != NULL);
	assert(size != 0);
	assert(buffer != NULL);
	assert(allocation != NULL);

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
	{
		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
			return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

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

			if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
				return OUT_OF_HOST_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
				return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
			else if (vkResult == VK_ERROR_MEMORY_MAP_FAILED)
				return FAILED_TO_MAP_MEMORY_MPGX_RESULT;
			else
				return UNKNOWN_ERROR_MPGX_RESULT;
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

				if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
					return OUT_OF_HOST_MEMORY_MPGX_RESULT;
				else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
					return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
				else
					return UNKNOWN_ERROR_MPGX_RESULT;
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

				if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
					return OUT_OF_HOST_MEMORY_MPGX_RESULT;
				else if (vkResult == VK_ERROR_OUT_OF_DEVICE_MEMORY)
					return OUT_OF_DEVICE_MEMORY_MPGX_RESULT;
				else if (vkResult == VK_ERROR_MEMORY_MAP_FAILED)
					return FAILED_TO_MAP_MEMORY_MPGX_RESULT;
				else
					return UNKNOWN_ERROR_MPGX_RESULT;
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
			return UNKNOWN_ERROR_MPGX_RESULT;
		}
	}

	*buffer = bufferInstance;
	*allocation = allocationInstance;

	if (mappedData != NULL)
		*mappedData = mappedBuffer;
	if (address != NULL)
		*address = bufferAddress;
	return SUCCESS_MPGX_RESULT;
}
inline static MpgxResult unmapVkRayTracingBuffer(
	VmaAllocator allocator,
	VmaAllocation allocation,
	size_t size)
{
	assert(allocator != NULL);
	assert(allocation != NULL);
	assert(size != 0);

	VkResult vkResult = vmaFlushAllocation(
		allocator,
		allocation,
		0,
		size);

	vmaUnmapMemory(
		allocator,
		allocation);

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

inline static MpgxResult createBuildVkAccelerationStructure(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
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
	assert(device != NULL);
	assert(allocator != NULL);
	assert(transferQueue != NULL);
	assert(transferCommandBuffer != NULL);
	assert(transferFence != NULL);
	assert(rayTracing != NULL);
	assert(buildGeometryInfo != NULL);
	assert(primitiveCount != 0);
	assert(buffer != NULL);
	assert(allocation != NULL);
	assert(accelerationStructure != NULL);

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

	MpgxResult mpgxResult = createVkRayTracingBuffer(
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

	if (mpgxResult != SUCCESS_MPGX_RESULT)
		return mpgxResult;

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

		if (vkResult == VK_ERROR_OUT_OF_HOST_MEMORY)
			return OUT_OF_HOST_MEMORY_MPGX_RESULT;
		else
			return UNKNOWN_ERROR_MPGX_RESULT;
	}

	VkBuffer scratchBuffer;
	VmaAllocation scratchAllocation;
	VkDeviceAddress scratchBufferAddress;

	mpgxResult = createVkRayTracingBuffer(
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

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		rayTracing->vk.destroyAccelerationStructure(
			device,
			accelerationStructureInstance,
			NULL);
		return mpgxResult;
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

	mpgxResult = beginVkOneTimeCommandBuffer(
		transferCommandBuffer);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
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
		return mpgxResult;
	}

	rayTracing->vk.cmdBuildAccelerationStructures(
		transferCommandBuffer,
		1,
		buildGeometryInfo,
		buildRangeInfos);

	mpgxResult = endSubmitWaitVkCommandBuffer(
		device,
		transferQueue,
		transferFence,
		transferCommandBuffer);

	vmaDestroyBuffer(
		allocator,
		scratchBuffer,
		scratchAllocation);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		vmaDestroyBuffer(
			allocator,
			bufferInstance,
			allocationInstance);
		rayTracing->vk.destroyAccelerationStructure(
			device,
			accelerationStructureInstance,
			NULL);
		return mpgxResult;
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
			return UNKNOWN_ERROR_MPGX_RESULT;
		}

		*deviceAddress = address;
	}

	*buffer = bufferInstance;
	*allocation = allocationInstance;
	*accelerationStructure = accelerationStructureInstance;
	return SUCCESS_MPGX_RESULT;
}

inline static void destroyVkRayTracingMesh(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayTracingMesh rayTracingMesh,
	bool destroyBuffers)
{
	assert(device != NULL);
	assert(allocator != NULL);
	assert(rayTracing != NULL);

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
inline static MpgxResult createVkRayTracingMesh(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	RayTracing rayTracing,
	Window window,
	size_t vertexStride,
	IndexType indexType,
	Buffer vertexBuffer,
	Buffer indexBuffer,
	RayTracingMesh* rayTracingMesh)
{
	assert(device != NULL);
	assert(allocator != NULL);
	assert(transferQueue != NULL);
	assert(transferCommandBuffer != NULL);
	assert(transferFence != NULL);
	assert(rayTracing != NULL);
	assert(window != NULL);
	assert(vertexStride != 0);
	assert(indexType < INDEX_TYPE_COUNT);
	assert(vertexBuffer != NULL);
	assert(indexBuffer != NULL);
	assert(rayTracingMesh != NULL);

	RayTracingMesh rayTracingMeshInstance = calloc(1,
		sizeof(RayTracingMesh_T));

	if (rayTracingMeshInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	rayTracingMeshInstance->vk.window = window;

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
		destroyVkRayTracingMesh(
			device,
			allocator,
			rayTracing,
			rayTracingMeshInstance,
			false);
		return VULKAN_IS_NOT_SUPPORTED_MPGX_RESULT;
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
			rayTracingMeshInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
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
			rayTracingMeshInstance,
			false);
		return UNKNOWN_ERROR_MPGX_RESULT;
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

	MpgxResult mpgxResult = createBuildVkAccelerationStructure(
		device,
		allocator,
		transferQueue,
		transferCommandBuffer,
		transferFence,
		rayTracing,
		VK_ACCELERATION_STRUCTURE_TYPE_BOTTOM_LEVEL_KHR,
		&buildGeometryInfo,
		primitiveCount,
		&buffer,
		&allocation,
		&accelerationStructure,
		&deviceAddress);

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkRayTracingMesh(
			device,
			allocator,
			rayTracing,
			rayTracingMeshInstance,
			false);
		return mpgxResult;
	}

	rayTracingMeshInstance->vk.buffer = buffer;
	rayTracingMeshInstance->vk.allocation = allocation;
	rayTracingMeshInstance->vk.accelerationStructure = accelerationStructure;
	rayTracingMeshInstance->vk.deviceAddress = deviceAddress;

	*rayTracingMesh = rayTracingMeshInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
