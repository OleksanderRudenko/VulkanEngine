#include "stdafx.h"
#include "swapchain.h"
#include "buffer.h"
#include "surface.h"
#include "texture.h"
#include "window.h"
#include <iostream>
#include <array>

namespace xengine
{

//======================================================================================================================
Swapchain::Swapchain(VkDevice				_logicalDevice,
					 VkPhysicalDevice		_physicalDevice,
					 Surface*				_surface,
					 std::shared_ptr<Window>	_window)
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
	VkSurfaceCapabilitiesKHR capabilities		= surface_->GetCapabilities(physicalDevice_);
	VkSurfaceFormatKHR		surfaceFormat		= Surface::ChooseSwapSurfaceFormat(surface_->GetFormats(physicalDevice_));
	VkPresentModeKHR		presentMode			= Surface::ChooseSwapPresentMode(surface_->GetPresentModes(physicalDevice_));
	VkExtent2D				extent				= Surface::ChooseSwapExtent(capabilities, *window_);

	uint32_t				imageCount			= surface_->GetCapabilities(physicalDevice_).minImageCount + 1;
	if (capabilities.maxImageCount > 0 && imageCount > capabilities.maxImageCount)
	{
		imageCount = capabilities.maxImageCount;
	}

	VkSwapchainCreateInfoKHR createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR;
	createInfo.surface			= surface_->GetSurface();
	createInfo.minImageCount	= imageCount;
	createInfo.imageFormat		= surfaceFormat.format;
	createInfo.imageColorSpace	= surfaceFormat.colorSpace;
	createInfo.imageExtent		= extent;
	createInfo.imageArrayLayers	= 1;
	createInfo.imageUsage		= VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT;

	QueueFamilyIndices indices = surface_->FindQueueFamilies(physicalDevice_);
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
bool Swapchain::CreateDepthImageViews()
{
	VkFormat depthFormat = FindDepthFormat(physicalDevice_);
	depthImages_.resize(images_.size());
	depthImageMemories_.resize(images_.size());
	depthImageViews_.resize(images_.size());

	for (size_t i = 0; i < images_.size(); ++i)
	{
		VkImageCreateInfo imageInfo{};
		imageInfo.sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
		imageInfo.imageType = VK_IMAGE_TYPE_2D;
		imageInfo.extent.width = extent_.width;
		imageInfo.extent.height = extent_.height;
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = depthFormat;
		imageInfo.tiling = VK_IMAGE_TILING_OPTIMAL;
		imageInfo.initialLayout = VK_IMAGE_LAYOUT_UNDEFINED;
		imageInfo.usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT;
		imageInfo.samples = VK_SAMPLE_COUNT_1_BIT;
		imageInfo.sharingMode = VK_SHARING_MODE_EXCLUSIVE;

		if (vkCreateImage(logicalDevice_, &imageInfo, nullptr, &depthImages_[i]) != VK_SUCCESS)
		{
			std::cout << "failed to create depth image!\n";
			return false;
		}

		VkMemoryRequirements memRequirements;
		vkGetImageMemoryRequirements(logicalDevice_, depthImages_[i], &memRequirements);

		VkMemoryAllocateInfo allocInfo{};
		allocInfo.sType = VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
		allocInfo.allocationSize = memRequirements.size;
		allocInfo.memoryTypeIndex = Buffer::FindMemoryType(physicalDevice_, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT).value();

		if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &depthImageMemories_[i]) != VK_SUCCESS)
		{
			std::cout << "failed to allocate depth image memory!\n";
			return false;
		}

		vkBindImageMemory(logicalDevice_, depthImages_[i], depthImageMemories_[i], 0);

		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = depthImages_[i];
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = depthFormat;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		if (vkCreateImageView(logicalDevice_, &viewInfo, nullptr, &depthImageViews_[i]) != VK_SUCCESS)
		{
			std::cout << "failed to create depth image view!\n";
			return false;
		}
	}
	return true;
}
//======================================================================================================================
bool Swapchain::CreateFramebuffers(VkRenderPass _renderPass)
{
	framebuffers_.resize(imageViews_.size());
	for (size_t i = 0; i < imageViews_.size(); ++i)
	{
		std::array<VkImageView, 2> attachments =
		{
			imageViews_[i],
			depthImageViews_[i]
		};

		VkFramebufferCreateInfo framebufferInfo{};
		framebufferInfo.sType			= VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO;
		framebufferInfo.renderPass		= _renderPass;
		framebufferInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
		framebufferInfo.pAttachments	= attachments.data();
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
	CreateImageViews();
	CreateDepthImageViews();
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

	// Destroy depth image views
	for (auto depthImageView : depthImageViews_)
	{
		vkDestroyImageView(logicalDevice_, depthImageView, nullptr);
	}

	// Destroy depth images
	for (auto depthImage : depthImages_)
	{
		vkDestroyImage(logicalDevice_, depthImage, nullptr);
	}

	// Free depth image memory
	for (auto depthImageMemory : depthImageMemories_)
	{
		vkFreeMemory(logicalDevice_, depthImageMemory, nullptr);
	}

	vkDestroySwapchainKHR(logicalDevice_, chain_, nullptr);
}

}