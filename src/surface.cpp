#include "stdafx.h"
#include "surface.h"
#include "instance.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Surface::Surface(std::reference_wrapper<Instance>	_instance,
				 GLFWwindow*						_window)
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
	if (glfwCreateWindowSurface(instance_.get(), window_, nullptr, &surface_) != VK_SUCCESS)
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

	if (formatCount != 0)
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

	if (presentModeCount != 0)
	{
		presentModes_.resize(presentModeCount);
		vkGetPhysicalDeviceSurfacePresentModesKHR(_device, surface_, &presentModeCount, presentModes_.data());
	}
	return presentModes_;
}

}