#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

class Swapchain
{
public:
	Swapchain(std::reference_wrapper<VkDevice> logicalDevice,
				  std::reference_wrapper<VkPhysicalDevice>);
	Swapchain(const Swapchain&)				= delete;
	Swapchain(Swapchain&&)					= delete;
	~Swapchain()							= default;

	Swapchain&	operator=(const Swapchain&)	= delete;
	Swapchain&	operator=(Swapchain&&)		= delete;

	bool		Create();

private:
	const std::reference_wrapper<VkDevice>				logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>		physicalDevice_;

	VkSwapchainKHR										swapChain_				= VK_NULL_HANDLE;
	std::vector<VkImage>								swapChainImages_;
	VkFormat											swapChainImageFormat_	= VK_FORMAT_UNDEFINED;
	VkExtent2D											swapChainExtent_;
	std::vector<VkImageView>							swapChainImageViews_;
	std::vector<VkFramebuffer>							swapChainFramebuffers_;
};