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
inline static MpgxResult createVkRayTracing(
	VkInstance instance,
	VkPhysicalDevice physicalDevice,
	RayTracing* rayTracing)
{
	assert(instance != NULL);
	assert(physicalDevice != NULL);
	assert(rayTracing != NULL);

	RayTracing rayTracingInstance = calloc(1,
		sizeof(RayTracing_T));

	if (rayTracingInstance == NULL)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

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

	rayTracingInstance->vk.rayTracingPipelineProperties = rayTracingPipelineProperties;

	RayTracingPipeline* pipelines = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingPipeline));

	if (pipelines == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.pipelines = pipelines;
	rayTracingInstance->vk.pipelineCount = MPGX_DEFAULT_CAPACITY;
	rayTracingInstance->vk.pipelineCapacity = 0;

	RayTracingMesh* meshes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingMesh));

	if (meshes == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.meshes = meshes;
	rayTracingInstance->vk.meshCount = MPGX_DEFAULT_CAPACITY;
	rayTracingInstance->vk.meshCapacity = 0;

	RayTracingScene* scenes = malloc(
		MPGX_DEFAULT_CAPACITY * sizeof(RayTracingScene));

	if (scenes == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.scenes = scenes;
	rayTracingInstance->vk.sceneCount = MPGX_DEFAULT_CAPACITY;
	rayTracingInstance->vk.sceneCapacity = 0;

	PFN_vkGetAccelerationStructureBuildSizesKHR
		getAccelerationStructureBuildSizes =
		(PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetInstanceProcAddr(
		instance, "vkGetAccelerationStructureBuildSizesKHR");

	if (getAccelerationStructureBuildSizes == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.getAccelerationStructureBuildSizes =
		getAccelerationStructureBuildSizes;

	PFN_vkCreateAccelerationStructureKHR
		createAccelerationStructure =
		(PFN_vkCreateAccelerationStructureKHR)vkGetInstanceProcAddr(
		instance, "vkCreateAccelerationStructureKHR");

	if (createAccelerationStructure == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.createAccelerationStructure =
		createAccelerationStructure;

	PFN_vkDestroyAccelerationStructureKHR
		destroyAccelerationStructure =
		(PFN_vkDestroyAccelerationStructureKHR)vkGetInstanceProcAddr(
		instance, "vkDestroyAccelerationStructureKHR");

	if (destroyAccelerationStructure == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.destroyAccelerationStructure =
		destroyAccelerationStructure;

	PFN_vkCmdBuildAccelerationStructuresKHR
		cmdBuildAccelerationStructures =
		(PFN_vkCmdBuildAccelerationStructuresKHR)vkGetInstanceProcAddr(
		instance, "vkCmdBuildAccelerationStructuresKHR");

	if (cmdBuildAccelerationStructures == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.cmdBuildAccelerationStructures =
		cmdBuildAccelerationStructures;

	PFN_vkGetAccelerationStructureDeviceAddressKHR
		getAccelerationStructureDeviceAddress =
		(PFN_vkGetAccelerationStructureDeviceAddressKHR)vkGetInstanceProcAddr(
		instance, "vkGetAccelerationStructureDeviceAddressKHR");

	if (getAccelerationStructureDeviceAddress == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.getAccelerationStructureDeviceAddress =
		getAccelerationStructureDeviceAddress;

	PFN_vkCreateRayTracingPipelinesKHR
		createRayTracingPipelinesKHR =
		(PFN_vkCreateRayTracingPipelinesKHR)vkGetInstanceProcAddr(
		instance, "vkCreateRayTracingPipelinesKHR");

	if (createRayTracingPipelinesKHR == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.createRayTracingPipelinesKHR =
		createRayTracingPipelinesKHR;

	PFN_vkGetRayTracingShaderGroupHandlesKHR
		getRayTracingShaderGroupHandles =
		(PFN_vkGetRayTracingShaderGroupHandlesKHR)vkGetInstanceProcAddr(
		instance, "vkGetRayTracingShaderGroupHandlesKHR");

	if (getRayTracingShaderGroupHandles == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.getRayTracingShaderGroupHandles =
		getRayTracingShaderGroupHandles;

	PFN_vkCmdTraceRaysKHR cmdTraceRays =
		(PFN_vkCmdTraceRaysKHR)vkGetInstanceProcAddr(
		instance, "vkCmdTraceRaysKHR");

	if (cmdTraceRays == NULL)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.cmdTraceRays = cmdTraceRays;

	*rayTracing = rayTracingInstance;
	return SUCCESS_MPGX_RESULT;
}
#endif

// TODO: cache ray tracing mesh and render:
// scratch, instance buffers
