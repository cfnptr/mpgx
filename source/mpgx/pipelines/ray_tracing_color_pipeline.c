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

#include "mpgx/pipelines/ray_tracing_color_pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/ray_tracing.h"

#include <string.h>

typedef struct RayGenPushConstants
{
	Mat4F invView;
	Mat4F invProj;
} RayGenPushConstants;
typedef struct BaseHandle
{
	Window window;
	RayTracingScene scene;
	RayGenPushConstants rgpc;
	Image storageImage;
} BaseHandle;
#if MPGX_SUPPORT_VULKAN
typedef struct VkHandle
{
	Window window;
	RayTracingScene scene;
	RayGenPushConstants rgpc;
	Image storageImage;
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
} VkHandle;
#endif
typedef union Handle_T
{
	BaseHandle base;
#if MPGX_SUPPORT_VULKAN
	VkHandle vk;
#endif
} Handle_T;

typedef Handle_T* Handle;

#if MPGX_SUPPORT_VULKAN
static const VkPushConstantRange pushConstantRanges[2] = {
	{
		VK_SHADER_STAGE_RAYGEN_BIT_KHR,
		0,
		sizeof(RayGenPushConstants),
	},
};

inline static VkDescriptorSetLayout createVkDescriptorSetLayout(
	VkDevice device)
{
	VkDescriptorSetLayoutBinding descriptorSetLayoutBindings[2] = {
		{
			0,
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			1,
			VK_SHADER_STAGE_RAYGEN_BIT_KHR,
			NULL,
		},
		{
			1,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			1,
			VK_SHADER_STAGE_RAYGEN_BIT_KHR,
			NULL,
		},
	};
	VkDescriptorSetLayoutCreateInfo descriptorSetLayoutCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
		NULL,
		0,
		2,
		descriptorSetLayoutBindings,
	};

	VkDescriptorSetLayout descriptorSetLayout;

	VkResult result = vkCreateDescriptorSetLayout(
		device,
		&descriptorSetLayoutCreateInfo,
		NULL,
		&descriptorSetLayout);

	if(result != VK_SUCCESS)
		return NULL;

	return descriptorSetLayout;
}
inline static VkDescriptorPool createVkDescriptorPool(
	VkDevice device)
{
	VkDescriptorPoolSize descriptorPoolSizes[2] = {
		{
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			1,
		},
		{
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			1,
		},
	};
	VkDescriptorPoolCreateInfo descriptorPoolCreateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
		NULL,
		0,
		1,
		2,
		descriptorPoolSizes,
	};

	VkDescriptorPool descriptorPool;

	VkResult result = vkCreateDescriptorPool(
		device,
		&descriptorPoolCreateInfo,
		NULL,
		&descriptorPool);

	if (result != VK_SUCCESS)
		return NULL;

	return descriptorPool;
}
inline static VkDescriptorSet createVkDescriptorSet(
	VkDevice device,
	VkDescriptorSetLayout descriptorSetLayout,
	VkDescriptorPool descriptorPool,
	VkAccelerationStructureKHR tlas,
	VkImageView storageImageView)
{
	VkDescriptorSetAllocateInfo descriptorSetAllocateInfo = {
		VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
		NULL,
		descriptorPool,
		1,
		&descriptorSetLayout,
	};

	VkDescriptorSet descriptorSet;

	VkResult result = vkAllocateDescriptorSets(
		device,
		&descriptorSetAllocateInfo,
		&descriptorSet);

	if (result != VK_SUCCESS)
		return NULL;

	VkWriteDescriptorSetAccelerationStructureKHR writeAccelerationStructure[1] = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET_ACCELERATION_STRUCTURE_KHR,
			NULL,
			1,
			&tlas
		},
	};
	VkDescriptorImageInfo storageDescriptorImageInfos[1] = {
		{
			NULL,
			storageImageView,
			VK_IMAGE_LAYOUT_GENERAL,
		},
	};

	VkWriteDescriptorSet writeDescriptorSets[2] = {
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			&writeAccelerationStructure,
			descriptorSet,
			0,
			0,
			1,
			VK_DESCRIPTOR_TYPE_ACCELERATION_STRUCTURE_KHR,
			NULL,
			NULL,
			NULL,
		},
		{
			VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
			NULL,
			descriptorSet,
			1,
			0,
			1,
			VK_DESCRIPTOR_TYPE_STORAGE_IMAGE,
			storageDescriptorImageInfos,
			NULL,
			NULL,
		},
	};

	vkUpdateDescriptorSets(
		device,
		2,
		writeDescriptorSets,
		0,
		NULL);

	return descriptorSet;
}

static void onVkBind(RayTracingPipeline rayTracingPipeline)
{
	Handle handle = rayTracingPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkCommandBuffer commandBuffer = vkWindow->currenCommandBuffer;

	// TODO: create storage image in the rayTracing struct or base pipeline
	VkImageMemoryBarrier imageMemoryBarrier = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_NONE_KHR,
		VK_ACCESS_TRANSFER_READ_BIT,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		handle->vk.storageImage->vk.handle,
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		},
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier);

	VkImageMemoryBarrier imageMemoryBarrier3 = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_NONE_KHR,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_IMAGE_LAYOUT_UNDEFINED,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		vkWindow->swapchain->buffers[
			vkWindow->bufferIndex].image,
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		},
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier3);

	VkImageCopy copyRegion = {
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		{ 0, 0, 0 },
		{ VK_IMAGE_ASPECT_COLOR_BIT, 0, 0, 1 },
		{ 0, 0, 0 },
		{ 1280, 720, 1 },
	};

	vkCmdCopyImage(
		commandBuffer,
		handle->vk.storageImage->vk.handle,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		vkWindow->swapchain->buffers[
			vkWindow->bufferIndex].image,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		1,
		&copyRegion);

	// TODO: possibly set this in image constructor
	VkImageMemoryBarrier imageMemoryBarrier2 = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_NONE_KHR,
		VK_ACCESS_MEMORY_WRITE_BIT,
		VK_IMAGE_LAYOUT_TRANSFER_SRC_OPTIMAL,
		VK_IMAGE_LAYOUT_GENERAL,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		handle->vk.storageImage->vk.handle,
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		},
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_RAY_TRACING_SHADER_BIT_KHR,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier2);

	VkImageMemoryBarrier imageMemoryBarrier4 = {
		VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
		NULL,
		VK_ACCESS_TRANSFER_WRITE_BIT,
		VK_ACCESS_NONE_KHR,
		VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
		VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
		VK_QUEUE_FAMILY_IGNORED,
		VK_QUEUE_FAMILY_IGNORED,
		vkWindow->swapchain->buffers[
			vkWindow->bufferIndex].image,
		{
			VK_IMAGE_ASPECT_COLOR_BIT,
			0,
			1,
			0,
			1,
		},
	};

	vkCmdPipelineBarrier(
		commandBuffer,
		VK_PIPELINE_STAGE_TRANSFER_BIT,
		VK_PIPELINE_STAGE_BOTTOM_OF_PIPE_BIT,
		0,
		0,
		NULL,
		0,
		NULL,
		1,
		&imageMemoryBarrier4);

	vkCmdBindDescriptorSets(
		commandBuffer,
		VK_PIPELINE_BIND_POINT_RAY_TRACING_KHR,
		rayTracingPipeline->vk.layout,
		0,
		1,
		&handle->vk.descriptorSet,
		0,
		NULL);
	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		rayTracingPipeline->vk.layout,
		VK_SHADER_STAGE_RAYGEN_BIT_KHR,
		0,
		sizeof(RayGenPushConstants),
		&handle->vk.rgpc);
}
static void onVkDestroy(void* _handle)
{
	Handle handle = _handle;
	VkWindow vkWindow = getVkWindow(handle->vk.window);
	VkDevice device = vkWindow->device;

	vkDestroyDescriptorPool(
		device,
		handle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		handle->vk.descriptorSetLayout,
		NULL);
	destroyImage(handle->vk.storageImage);
	free(handle);
}
inline static RayTracingPipeline createVkPipeline(
	Window window,
	VkAccelerationStructureKHR tlas,
	VkImageView storageImageView,
	Handle handle,
	Shader* generationShaders,
	size_t generationShaderCount,
	Shader* missShaders,
	size_t missShaderCount,
	Shader* closestHitShaders,
	size_t closestHitShaderCount)
{
	VkWindow vkWindow = getVkWindow(window);
	VkDevice device = vkWindow->device;

	VkDescriptorSetLayout descriptorSetLayout =
		createVkDescriptorSetLayout(device);

	if (descriptorSetLayout == NULL)
	{
		free(handle);
		return NULL;
	}

	VkRayTracingPipelineCreateData createData = {
		1,
		&descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	VkDescriptorPool descriptorPool = createVkDescriptorPool(device);

	if (descriptorPool == NULL)
	{
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	VkDescriptorSet descriptorSet = createVkDescriptorSet(
		device,
		descriptorSetLayout,
		descriptorPool,
		tlas,
		storageImageView);

	if (descriptorSet == NULL)
	{
		vkDestroyDescriptorPool(
			device,
			descriptorPool,
			NULL);
		vkDestroyDescriptorSetLayout(
			device,
			descriptorSetLayout,
			NULL);
		free(handle);
		return NULL;
	}

	handle->vk.descriptorSetLayout = descriptorSetLayout;
	handle->vk.descriptorPool = descriptorPool;
	handle->vk.descriptorSet = descriptorSet;

	return createRayTracingPipeline(
		window,
		RAY_TRACING_COLOR_PIPELINE_NAME,
		onVkBind,
		onVkDestroy,
		handle,
		&createData,
		generationShaders,
		generationShaderCount,
		missShaders,
		missShaderCount,
		closestHitShaders,
		closestHitShaderCount);
}
#endif

RayTracingPipeline createRayTracingColorPipeline(
	Window window,
	Shader generationShader,
	Shader missShader,
	Shader closestHitShader,
	RayTracingScene scene)
{
	assert(window != NULL);
	assert(generationShader != NULL);
	assert(missShader != NULL);
	assert(missShader != NULL);
	assert(scene != NULL);
	assert(generationShader->base.type == RAY_GENERATION_SHADER_TYPE);
	assert(missShader->base.type == RAY_MISS_SHADER_TYPE);
	assert(closestHitShader->base.type == RAY_CLOSEST_HIT_SHADER_TYPE);
	assert(generationShader->base.window == window);
	assert(missShader->base.window == window);
	assert(closestHitShader->base.window == window);
	assert(scene->base.window == window);

	Handle handle = malloc(sizeof(Handle_T));

	if (handle == NULL)
		return NULL;

	handle->base.window = window;
	handle->base.scene = scene;
	handle->base.rgpc.invProj = identMat4F;
	handle->base.rgpc.invProj = identMat4F;

	const void* data = NULL;

	Vec2U framebufferSize = getFramebufferSize(
		getWindowFramebuffer(window));
	Vec3U size = vec3U(framebufferSize.x, framebufferSize.y, 1);

	Image storageImage = createImage(
		window,
		STORAGE_IMAGE_TYPE,
		IMAGE_2D,
		R8G8B8A8_UNORM_IMAGE_FORMAT,
		&data,
		size,
		1,
		true);

	if (storageImage == NULL)
	{
		free(handle);
		return NULL;
	}

	handle->base.storageImage = storageImage;

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkPipeline(
			window,
			scene->vk.accelerationStructure,
			storageImage->vk.imageView,
			handle,
			&generationShader,
			1,
			&missShader,
			1,
			&closestHitShader,
			1);
#else
		abort();
#endif
	}
	else
	{
		abort();
	}
}

RayTracingScene getRayTracingColorPipelineScene(
	RayTracingPipeline rayTracingPipeline)
{
	assert(rayTracingPipeline != NULL);
	assert(strcmp(rayTracingPipeline->base.name,
		RAY_TRACING_COLOR_PIPELINE_NAME) == 0);
	Handle handle = rayTracingPipeline->base.handle;
	return handle->base.scene;
}

Mat4F getRayTracingColorPipelineInvView(
	RayTracingPipeline rayTracingPipeline)
{
	assert(rayTracingPipeline != NULL);
	assert(strcmp(rayTracingPipeline->base.name,
		RAY_TRACING_COLOR_PIPELINE_NAME) == 0);
	Handle handle = rayTracingPipeline->base.handle;
	return handle->base.rgpc.invView;
}
void setRayTracingColorPipelineInvView(
	RayTracingPipeline rayTracingPipeline,
	Mat4F invView)
{
	assert(rayTracingPipeline != NULL);
	assert(strcmp(rayTracingPipeline->base.name,
		RAY_TRACING_COLOR_PIPELINE_NAME) == 0);
	Handle handle = rayTracingPipeline->base.handle;
	handle->base.rgpc.invView = invView;
}

Mat4F getRayTracingColorPipelineInvProj(
	RayTracingPipeline rayTracingPipeline)
{
	assert(rayTracingPipeline != NULL);
	assert(strcmp(rayTracingPipeline->base.name,
		RAY_TRACING_COLOR_PIPELINE_NAME) == 0);
	Handle handle = rayTracingPipeline->base.handle;
	return handle->base.rgpc.invProj;
}
void setRayTracingColorPipelineInvProj(
	RayTracingPipeline rayTracingPipeline,
	Mat4F invProj)
{
	assert(rayTracingPipeline != NULL);
	assert(strcmp(rayTracingPipeline->base.name,
		RAY_TRACING_COLOR_PIPELINE_NAME) == 0);
	Handle handle = rayTracingPipeline->base.handle;
	handle->base.rgpc.invProj = invProj;
}
