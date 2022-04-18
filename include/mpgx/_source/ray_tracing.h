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
#ifndef NDEBUG
	bool isEnumeratingPipelines;
	bool isEnumeratingMeshes;
	bool isEnumeratingScenes;
#endif
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
#ifndef NDEBUG
	bool isEnumeratingPipelines;
	bool isEnumeratingMeshes;
	bool isEnumeratingScenes;
	uint8_t _alignment[5];
#endif
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
	if (!rayTracing)
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
	assert(instance);
	assert(physicalDevice);
	assert(rayTracing);

	RayTracing rayTracingInstance = calloc(1,
		sizeof(RayTracing_T));

	if (!rayTracingInstance)
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;

#ifndef NDEBUG
	rayTracingInstance->vk.isEnumeratingPipelines = false;
	rayTracingInstance->vk.isEnumeratingMeshes = false;
	rayTracingInstance->vk.isEnumeratingScenes = false;
#endif

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
		 sizeof(RayTracingPipeline));

	if (!pipelines)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.pipelines = pipelines;
	rayTracingInstance->vk.pipelineCapacity = 1;
	rayTracingInstance->vk.pipelineCount = 0;

	RayTracingMesh* meshes = malloc(
		sizeof(RayTracingMesh));

	if (!meshes)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.meshes = meshes;
	rayTracingInstance->vk.meshCapacity = 1;
	rayTracingInstance->vk.meshCount = 0;

	RayTracingScene* scenes = malloc(
		sizeof(RayTracingScene));

	if (!scenes)
	{
		destroyVkRayTracing(rayTracingInstance);
		return OUT_OF_HOST_MEMORY_MPGX_RESULT;
	}

	rayTracingInstance->vk.scenes = scenes;
	rayTracingInstance->vk.sceneCapacity = 1;
	rayTracingInstance->vk.sceneCount = 0;

	PFN_vkGetAccelerationStructureBuildSizesKHR
		getAccelerationStructureBuildSizes =
		(PFN_vkGetAccelerationStructureBuildSizesKHR)vkGetInstanceProcAddr(
		instance, "vkGetAccelerationStructureBuildSizesKHR");

	if (!getAccelerationStructureBuildSizes)
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

	if (!createAccelerationStructure)
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

	if (!destroyAccelerationStructure)
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

	if (!cmdBuildAccelerationStructures)
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

	if (!getAccelerationStructureDeviceAddress)
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

	if (!createRayTracingPipelinesKHR)
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

	if (!getRayTracingShaderGroupHandles)
	{
		destroyVkRayTracing(rayTracingInstance);
		return FAILED_TO_GET_FUNCTION_ADDRESS_MPGX_RESULT;
	}

	rayTracingInstance->vk.getRayTracingShaderGroupHandles =
		getRayTracingShaderGroupHandles;

	PFN_vkCmdTraceRaysKHR cmdTraceRays =
		(PFN_vkCmdTraceRaysKHR)vkGetInstanceProcAddr(
		instance, "vkCmdTraceRaysKHR");

	if (!cmdTraceRays)
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
