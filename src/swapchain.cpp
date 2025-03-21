#include "stdafx.h"
#include "swapchain.h"
#include "surface.h"
#include "texture.h"
#include "window.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Swapchain::Swapchain(std::reference_wrapper<VkDevice>			_logicalDevice,
					 std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
					 std::reference_wrapper<Surface>			_surface,
					 std::shared_ptr<Window>					_window)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, surface_(_surface)
, window_(_window)
{}
//======================================================================================================================
Swapchain::~Swapchain()
{
	Cleanup();
}
//======================================================================================================================
bool Swapchain::Create()
{
	VkSurfaceCapabilitiesKHR capabilities		= surface_.get().GetCapabilities(std::ref(physicalDevice_));
	VkSurfaceFormatKHR		surfaceFormat		= Surface::ChooseSwapSurfaceFormat(surface_.get().GetFormats(std::ref(physicalDevice_)));
	VkPresentModeKHR		presentMode			= Surface::ChooseSwapPresentMode(surface_.get().GetPresentModes(std::ref(physicalDevice_)));
	VkExtent2D				extent				= Surface::ChooseSwapExtent(capabilities, *window_);

	uint32_t				imageCount			= surface_.get().GetCapabilities(std::ref(physicalDevice_)).minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface			= surface_.get().GetSurface();
	createInfo.minImageCount	= imageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= extent;
	createInfo.imageArrayLayers	= 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = surface_.get().FindQueueFamilies(physicalDevice_);
	uint32_t queueFamilyIndices[] = {indices.graphicsFamily.value(), indices.presentFamily.value()};

	if (indices.graphicsFamily != indices.presentFamily) {
		createInfo.imageSharingMode			= VK_SHARING_MODE_CONCURRENT;
		createInfo.queueFamilyIndexCount	= 2;
		createInfo.pQueueFamilyIndices		= queueFamilyIndices;
	}
	else
	{
		createInfo.imageSharingMode			= VK_SHARING_MODE_EXCLUSIVE;
		createInfo.queueFamilyIndexCount	= 0;
		createInfo.pQueueFamilyIndices		= nullptr;
	}

	createInfo.preTransform		= capabilities.currentTransform;
	createInfo.compositeAlpha	= VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR;
	createInfo.presentMode		= presentMode;
	createInfo.clipped			= VK_TRUE;
	createInfo.oldSwapchain		= VK_NULL_HANDLE;

	//debug
	std::cout << "Min Image Count: " << createInfo.minImageCount << std::endl;
	std::cout << "Image Format: " << createInfo.imageFormat << std::endl;
	std::cout << "Image Extent: " << createInfo.imageExtent.width << "x" << createInfo.imageExtent.height << std::endl;

	chain_ = VK_NULL_HANDLE;
	VkResult result = vkCreateSwapchainKHR(logicalDevice_, &createInfo, nullptr, &chain_);
	if (result != VK_SUCCESS)
	{
		std::cout << "failed to create swap chain!\n";
		return false;
	}

	vkGetSwapchainImagesKHR(logicalDevice_, chain_, &imageCount, nullptr);
	images_.resize(imageCount);
	vkGetSwapchainImagesKHR(logicalDevice_, chain_, &imageCount, images_.data());

	imageFormat_	= surfaceFormat.format;
	extent_			= extent;

	return true;
}
//======================================================================================================================
bool Swapchain::CreateImageViews()
{
	imageViews_.resize(images_.size());

	for (size_t i = 0; i < images_.size(); ++i)
	{
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image								= images_.at(i);
		viewInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format								= VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel		= 0;
		viewInfo.subresourceRange.levelCount		= 1;
		viewInfo.subresourceRange.baseArrayLayer	= 0;
		viewInfo.subresourceRange.layerCount		= 1;

		VkImageView imageView;
		if (vkCreateImageView(logicalDevice_, &viewInfo, nullptr, &imageView) != VK_SUCCESS)
		{
			std::cout<< "failed to create texture image view!\n";
			return false;
		}

		imageViews_[i] = imageView;
	}
	return true;
}
//======================================================================================================================
bool Swapchain::CreateFramebuffers(VkRenderPass _renderPass)
{
	framebuffers_.resize(imageViews_.size());
	for (size_t i = 0; i < imageViews_.size(); ++i)
	{
		VkImageView attachments[] = { imageViews_[i] };

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= _renderPass;
		framebufferInfo.attachmentCount	= 1;
		framebufferInfo.pAttachments	= attachments;
		framebufferInfo.width			= extent_.width;
		framebufferInfo.height			= extent_.height;
		framebufferInfo.layers			= 1;

		if (vkCreateFramebuffer(logicalDevice_, &framebufferInfo, nullptr, &framebuffers_[i]) != VK_SUCCESS)
		{
			std::cout << "failed to create framebuffer!\n";
			return false;
		}
	}
	return true;
}
//======================================================================================================================
void Swapchain::Recreate(VkRenderPass _renderPass)
{
	int width = 0, height = 0;
	glfwGetFramebufferSize(window_->GetWindow(), &width, &height);
	while (width == 0 || height == 0)
	{
		glfwGetFramebufferSize(window_->GetWindow(), &width, &height);
		glfwWaitEvents();
	}

	vkDeviceWaitIdle(logicalDevice_);
	
	Cleanup();
	Create();
	//CreateImageViews();
	CreateFramebuffers(_renderPass);
}
//======================================================================================================================
void Swapchain::Cleanup()
{
	for (auto framebuffer : framebuffers_)
	{
		vkDestroyFramebuffer(logicalDevice_, framebuffer, nullptr);
	}

	for (auto imageView : imageViews_)
	{
		vkDestroyImageView(logicalDevice_, imageView, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice_, chain_, nullptr);
}

}