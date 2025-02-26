#pragma once

#include <vulkan/vulkan.h>

//**********************************************************************************************************************
//	QueueFamilyIndices
//----------------------------------------------------------------------------------------------------------------------
struct QueueFamilyIndices
{
	optional<uint32_t> graphicsFamily;
	optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

//**********************************************************************************************************************
//	SwapChainSupportDetails
//----------------------------------------------------------------------------------------------------------------------
struct SwapChainSupportDetails
{
	VkSurfaceCapabilitiesKHR	capabilities;
	vector<VkSurfaceFormatKHR>	formats;
	vector<VkPresentModeKHR>	presentModes;
};