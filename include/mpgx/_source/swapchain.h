#pragma once

struct VkSwapchain
{

};

typedef struct VkSwapchain* VkSwapchain;

inline static VkSwapchain createVkSwapchain()
{
	// TODO:
}
inline static void destroyVkSwapchain(
	VkSwapchain swapchain)
{
	if (swapchain == NULL)
		return;

	// TODO:
	free(swapchain);
}
