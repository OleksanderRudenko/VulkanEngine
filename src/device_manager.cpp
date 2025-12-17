#include "stdafx.h"
#include "device_manager.h"
#include "instance.h"
#include "surface.h"
#include <iostream>
#include <set>

namespace xengine
{

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

//======================================================================================================================
DeviceManager::DeviceManager(Instance* _instance,
							 Surface* _surface)
: instance_(_instance)
, surface_(_surface)
{}
//======================================================================================================================
DeviceManager::~DeviceManager()
{
	if (logicalDevice_ != VK_NULL_HANDLE)
	{
		vkDestroyDevice(logicalDevice_, nullptr);
	}
}
//======================================================================================================================
bool DeviceManager::Create()
{
	if (!PickPhysicalDevice())
	{
		return false;
	}
	if (!CreateLogicalDevice())
	{
		return false;
	}
	return true;
}
//======================================================================================================================
bool DeviceManager::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_->GetInstance(), &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		std::cout << "failed to find GPUs with Vulkan support!\n";
		return false;
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_->GetInstance(), &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		indices_ = surface_->FindQueueFamilies(device);
		if (IsDeviceSuitable(device))
		{
			physicalDevice_ = device;
			break;
		}
	}

	if (physicalDevice_ == VK_NULL_HANDLE)
	{
		std::cout << "failed to find a suitable GPU!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool DeviceManager::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices_.graphicsFamily.value(), indices_.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex	= queueFamily;
		queueCreateInfo.queueCount			= 1;
		queueCreateInfo.pQueuePriorities	= &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (instance_->IsValidationLayersEnabled())
	{
		deviceCreateInfo.enabledLayerCount		= static_cast<uint32_t>(instance_->GetValidationLayers().size());
		deviceCreateInfo.ppEnabledLayerNames	= instance_->GetValidationLayers().data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
	{
		std::cout << "failed to create logical device!\n";
		return false;
	}

	vkGetDeviceQueue(logicalDevice_, indices_.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, indices_.presentFamily.value(), 0, &presentQueue_);
	return true;
}
//======================================================================================================================
bool DeviceManager::IsDeviceSuitable(VkPhysicalDevice _device)
{
	bool extensionsSupported	= CheckDeviceExtensionSupport(_device);

	bool swapChainAdequate		= false;
	if (extensionsSupported)
	{
		std::vector<VkSurfaceFormatKHR>	formats			= surface_->GetFormats(_device);
		std::vector<VkPresentModeKHR>	presentModes	= surface_->GetPresentModes(_device);
		swapChainAdequate = !formats.empty() && !presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(_device, &supportedFeatures);

	return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
//======================================================================================================================
bool DeviceManager::CheckDeviceExtensionSupport(VkPhysicalDevice _device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		std::cout << "Available extension: " << extension.extensionName << "\n";
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

}
