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
#include "mpgx/_source/ray_tracing_mesh.h"

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

#if MPGX_SUPPORT_VULKAN
inline static void destroyVkRayTracingScene(
	VkDevice device,
	VmaAllocator allocator,
	RayTracing rayTracing,
	RayTracingScene rayTracingScene)
{
	assert(device);
	assert(allocator);
	assert(rayTracing);

	if (!rayTracingScene)
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
inline static MpgxResult createVkRayTracingScene(
	VkDevice device,
	VmaAllocator allocator,
	VkQueue transferQueue,
	VkCommandBuffer transferCommandBuffer,
	VkFence transferFence,
	RayTracing rayTracing,
	Window window,
	RayTracingMesh* meshes,
	size_t meshCount,
	RayTracingScene* rayTracingScene)
{
	assert(device);
	assert(allocator);
	assert(transferQueue);
	assert(transferCommandBuffer);
	assert(transferFence);
	assert(rayTracing);
	assert(window);
	assert(meshes);
	assert(meshCount > 0);
	assert(rayTracingScene);

	RayTracingScene rayTracingSceneInstance = calloc(1,
		sizeof(RayTracingScene_T));

	if (!rayTracingSceneInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

	rayTracingSceneInstance->vk.window = window;

	RayTracingMesh* rayTracingMeshes = malloc(
		meshCount * sizeof(RayTracingMesh));

	if (!meshes)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingSceneInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingSceneInstance->vk.meshes = rayTracingMeshes;
	rayTracingSceneInstance->vk.meshCount = meshCount;

	for (size_t i = 0; i < meshCount; i++)
		rayTracingMeshes[i] = meshes[i];

	size_t instanceBufferSize = meshCount *
		sizeof(VkAccelerationStructureInstanceKHR);
	VkAccelerationStructureInstanceKHR* instances =
		malloc(instanceBufferSize);

	if (!instances)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingSceneInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
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

	MpgxResult mpgxResult = createVkRayTracingBuffer(
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

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingSceneInstance);
		return mpgxResult;
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

	mpgxResult = createBuildVkAccelerationStructure(
		device,
		allocator,
		transferQueue,
		transferCommandBuffer,
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

	if (mpgxResult != SUCCESS_MPGX_RESULT)
	{
		destroyVkRayTracingScene(
			device,
			allocator,
			rayTracing,
			rayTracingSceneInstance);
		return mpgxResult;
	}

	rayTracingSceneInstance->vk.buffer = buffer;
	rayTracingSceneInstance->vk.allocation = allocation;
	rayTracingSceneInstance->vk.accelerationStructure = accelerationStructure;

	*rayTracingScene = rayTracingSceneInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif
