#define NOMINMAX

#include "stdafx.h"
#include "surface.h"
#include "instance.h"
#include "window.h"
#include <iostream>
#include <functional>
#include <algorithm>
#include <limits>

namespace xengine
{

//======================================================================================================================
Surface::Surface(std::reference_wrapper<Instance>	_instance,
				 GLFWwindow* _window)
	: instance_(_instance)
	, window_(_window)
{}
//======================================================================================================================
Surface::~Surface()
{
	vkDestroySurfaceKHR(instance_.get(), surface_, nullptr);
}
//======================================================================================================================
bool Surface::Create()
{
	if(glfwCreateWindowSurface(instance_.get(), window_, nullptr, &surface_) != VK_SUCCESS)
	{
		std::cout << "failed to create window surface!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
VkSurfaceCapabilitiesKHR Surface::GetCapabilities(std::reference_wrapper<VkPhysicalDevice>	_device)
{
	vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_device, surface_, &capabilities_);
	return capabilities_;
}
//======================================================================================================================
std::vector<VkSurfaceFormatKHR> Surface::GetFormats(std::reference_wrapper<VkPhysicalDevice>	_device)
{
	formats_.clear();
	uint32_t formatCount;
	vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface_, &formatCount, nullptr);

	if(formatCount != 0)
	{
		formats_.resize(formatCount);
		vkGetPhysicalDeviceSurfaceFormatsKHR(_device, surface_, &formatCount, formats_.data());
	}
	return formats_;
}
//======================================================================================================================
std::vector<VkPresentModeKHR> Surface::GetPresentModes(std::reference_wrapper<VkPhysicalDevice>	_device)
{
	presentModes_.clear();
	uint32_t presentModeCount;
	vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface_, &presentModeCount, nullptr);

	if(presentModeCount != 0)
	{
		presentModes_.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface_, &presentModeCount, presentModes_.data());
	}
	return presentModes_;
}
//======================================================================================================================
QueueFamilyIndices Surface::FindQueueFamilies(VkPhysicalDevice _device)
{
	QueueFamilyIndices indices;

	uint32_t queueFamilyCount = 0;
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, nullptr);

	std::vector<VkQueueFamilyProperties> queueFamilies(queueFamilyCount);
	vkGetPhysicalDeviceQueueFamilyProperties(_device, &queueFamilyCount, queueFamilies.data());

	int i = 0;
	for(const auto& queueFamily : queueFamilies)
	{
		if(queueFamily.queueFlags & VK_QUEUE_GRAPHICS_BIT)
		{
			indices.graphicsFamily = i;
		}

		VkBool32 presentSupport = false;
		vkGetPhysicalDeviceSurfaceSupportKHR(_device, i, surface_, &presentSupport);

		if(presentSupport)
		{
			indices.presentFamily = i;
		}

		if(indices.isComplete())
		{
			break;
		}

		++i;
	}

	return indices;
}
//======================================================================================================================
VkSurfaceFormatKHR Surface::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
	for(const auto& availableFormat : _availableFormats)
	{
		if(availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return _availableFormats[0];
}
//======================================================================================================================
VkPresentModeKHR Surface::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes)
{
	for(const auto& availablePresentMode : _availablePresentModes)
	{
		if(availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}

//======================================================================================================================
VkExtent2D Surface::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities,
									 const Window& _window)
{
	if(_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return _capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(_window.GetWindow(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width = std::clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
		actualExtent.height = std::clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);

		return actualExtent;
	}
}

}