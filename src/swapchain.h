#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace xengine
{

class Surface;
class Window;

class ENGINE_API Swapchain
{
public:
	Swapchain(std::reference_wrapper<VkDevice> logicalDevice,
			  std::reference_wrapper<VkPhysicalDevice>,
			  std::reference_wrapper<Surface>,
			  std::shared_ptr<Window>);
	Swapchain(const Swapchain&)				= delete;
	Swapchain(Swapchain&&)					= delete;
	~Swapchain();

	Swapchain&	operator=(const Swapchain&)	= delete;
	Swapchain&	operator=(Swapchain&&)		= delete;

	bool		Create();
	bool		CreateImageViews();
	bool		CreateFramebuffers(VkRenderPass);
	void		Recreate(VkRenderPass);

	const VkSwapchainKHR&				GetSwapChain()				const { return chain_; }
	const std::vector<VkFramebuffer>&	GetSwapChainFramebuffers()	const { return framebuffers_; }
	VkFormat							GetSwapChainImageFormat()	const { return imageFormat_; }
	VkExtent2D							GetSwapChainExtent()		const { return extent_; }

private:
	void		Cleanup();

	const std::reference_wrapper<VkDevice>			logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>	physicalDevice_;
	const std::reference_wrapper<Surface>			surface_;
	const std::shared_ptr<Window>					window_;

	VkSwapchainKHR									chain_			= VK_NULL_HANDLE;
	VkFormat										imageFormat_	= VK_FORMAT_UNDEFINED;
	VkExtent2D										extent_;
	std::vector<VkImage>							images_;
	std::vector<VkImageView>						imageViews_;
	std::vector<VkFramebuffer>						framebuffers_;
};

}