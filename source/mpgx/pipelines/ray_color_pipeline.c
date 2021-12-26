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

#include "mpgx/pipelines/ray_color_pipeline.h"
#include "mpgx/_source/image.h"
#include "mpgx/_source/window.h"
#include "mpgx/_source/ray_tracing.h"

#include <string.h>

typedef struct RayGenPushConstants
{
	Mat4F invView;
	Mat4F invProj;
} RayGenPushConstants;
typedef struct BasePipelineHandle
{
	Window window;
	RayScene rayScene;
	RayGenPushConstants rgpc;
	Image storageImage;
} BasePipelineHandle;
typedef struct VkPipelineHandle
{
	Window window;
	RayScene rayScene;
	RayGenPushConstants rgpc;
	Image storageImage;
#if MPGX_SUPPORT_VULKAN
	VkDescriptorSetLayout descriptorSetLayout;
	VkDescriptorPool descriptorPool;
	VkDescriptorSet descriptorSet;
#endif
} VkPipelineHandle;
union PipelineHandle_T
{
	BasePipelineHandle base;
	VkPipelineHandle vk;
};

typedef union PipelineHandle_T PipelineHandle_T;
typedef PipelineHandle_T* PipelineHandle;

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

static void onVkBind(RayPipeline rayPipeline)
{
	PipelineHandle pipelineHandle = rayPipeline->vk.handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
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
		pipelineHandle->vk.storageImage->vk.handle,
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
		pipelineHandle->vk.storageImage->vk.handle,
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
		pipelineHandle->vk.storageImage->vk.handle,
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
		rayPipeline->vk.layout,
		0,
		1,
		&pipelineHandle->vk.descriptorSet,
		0,
		NULL);
	vkCmdPushConstants(
		vkWindow->currenCommandBuffer,
		rayPipeline->vk.layout,
		VK_SHADER_STAGE_RAYGEN_BIT_KHR,
		0,
		sizeof(RayGenPushConstants),
		&pipelineHandle->vk.rgpc);
}
static bool onVkResize(
	RayPipeline rayPipeline,
	Vec2U newSize,
	void* createInfo)
{
	PipelineHandle pipelineHandle = rayPipeline->vk.handle;

	// TODO: resize

	VkRayPipelineCreateInfo _createInfo = {
		1,
		&pipelineHandle->vk.descriptorSetLayout,
		1,
		pushConstantRanges,
	};

	*(VkRayPipelineCreateInfo*)createInfo = _createInfo;
	return true;
}
static void onVkDestroy(void* handle)
{
	PipelineHandle pipelineHandle = handle;
	VkWindow vkWindow = getVkWindow(pipelineHandle->vk.window);
	VkDevice device = vkWindow->device;

	vkDestroyDescriptorPool(
		device,
		pipelineHandle->vk.descriptorPool,
		NULL);
	vkDestroyDescriptorSetLayout(
		device,
		pipelineHandle->vk.descriptorSetLayout,
		NULL);
	destroyImage(pipelineHandle->vk.storageImage);
	free(pipelineHandle);
}
inline static RayPipeline createVkHandle(
	Window window,
	VkAccelerationStructureKHR tlas,
	VkImageView storageImageView,
	PipelineHandle pipelineHandle,
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
		free(pipelineHandle);
		return NULL;
	}

	VkRayPipelineCreateInfo createInfo = {
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
		free(pipelineHandle);
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
		free(pipelineHandle);
		return NULL;
	}

	pipelineHandle->vk.descriptorSetLayout = descriptorSetLayout;
	pipelineHandle->vk.descriptorPool = descriptorPool;
	pipelineHandle->vk.descriptorSet = descriptorSet;

	return createRayPipeline(
		window,
		RAY_COLOR_PIPELINE_NAME,
		onVkBind,
		onVkResize,
		onVkDestroy,
		pipelineHandle,
		&createInfo,
		generationShaders,
		generationShaderCount,
		missShaders,
		missShaderCount,
		closestHitShaders,
		closestHitShaderCount);
}
#endif

RayPipeline createRayColorPipeline(
	Window window,
	Shader generationShader,
	Shader missShader,
	Shader closestHitShader,
	RayScene rayScene)
{
	assert(window != NULL);
	assert(generationShader != NULL);
	assert(missShader != NULL);
	assert(missShader != NULL);
	assert(rayScene != NULL);
	assert(generationShader->base.type == RAY_GENERATION_SHADER_TYPE);
	assert(missShader->base.type == RAY_MISS_SHADER_TYPE);
	assert(closestHitShader->base.type == RAY_CLOSEST_HIT_SHADER_TYPE);
	assert(generationShader->base.window == window);
	assert(missShader->base.window == window);
	assert(closestHitShader->base.window == window);
	assert(rayScene->base.window == window);

	PipelineHandle pipelineHandle = malloc(
		sizeof(PipelineHandle_T));

	if (pipelineHandle == NULL)
		return NULL;

	pipelineHandle->base.window = window;
	pipelineHandle->base.rayScene = rayScene;
	pipelineHandle->base.rgpc.invProj = identMat4F;
	pipelineHandle->base.rgpc.invProj = identMat4F;

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
		free(pipelineHandle);
		return NULL;
	}

	pipelineHandle->base.storageImage = storageImage;

	GraphicsAPI api = getWindowGraphicsAPI(window);

	if (api == VULKAN_GRAPHICS_API)
	{
#if MPGX_SUPPORT_VULKAN
		return createVkHandle(
			window,
			rayScene->vk.accelerationStructure,
			storageImage->vk.imageView,
			pipelineHandle,
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

RayScene getRayColorPipelineScene(
	RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	assert(strcmp(
		rayPipeline->base.name,
		RAY_COLOR_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		rayPipeline->base.handle;
	return pipelineHandle->base.rayScene;
}

Mat4F getColorRayPipelineInvView(
	RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	assert(strcmp(
		rayPipeline->base.name,
		RAY_COLOR_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		rayPipeline->base.handle;
	return pipelineHandle->base.rgpc.invView;
}
void setColorRayPipelineInvView(
	RayPipeline rayPipeline,
	Mat4F invView)
{
	assert(rayPipeline != NULL);
	assert(strcmp(
		rayPipeline->base.name,
		RAY_COLOR_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		rayPipeline->base.handle;
	pipelineHandle->base.rgpc.invView = invView;
}

Mat4F getColorRayPipelineInvProj(
	RayPipeline rayPipeline)
{
	assert(rayPipeline != NULL);
	assert(strcmp(
		rayPipeline->base.name,
		RAY_COLOR_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		rayPipeline->base.handle;
	return pipelineHandle->base.rgpc.invProj;
}
void setColorRayPipelineInvProj(
	RayPipeline rayPipeline,
	Mat4F invProj)
{
	assert(rayPipeline != NULL);
	assert(strcmp(
		rayPipeline->base.name,
		RAY_COLOR_PIPELINE_NAME) == 0);
	PipelineHandle pipelineHandle =
		rayPipeline->base.handle;
	pipelineHandle->base.rgpc.invProj = invProj;
}
