#include "stdafx.h"
#include "tools.h"

namespace xengine
{

std::vector<char> ReadFile(const std::string& _filename, std::ios_base::openmode _mode)
{
	std::ifstream file(_filename, _mode);

	if (!file.is_open())
	{
		throw std::runtime_error("failed to open file!");
	}

	size_t fileSize = (size_t) file.tellg();
	std::vector<char> buffer(fileSize);

	file.seekg(0);
	file.read(buffer.data(), fileSize);

	file.close();

	return buffer;
}
//======================================================================================================================
VkFormat FindSupportedFormat(std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
							 const std::vector<VkFormat>&				_candidates,
							 VkImageTiling								_tiling,
							 VkFormatFeatureFlags						_features)
{
	for (VkFormat format : _candidates)
	{
		VkFormatProperties props;
		vkGetPhysicalDeviceFormatProperties(_physicalDevice, format, &props);
		if (_tiling == VK_IMAGE_TILING_LINEAR && (props.linearTilingFeatures & _features) == _features)
		{
			return format;
		}
		else if (_tiling == VK_IMAGE_TILING_OPTIMAL && (props.optimalTilingFeatures & _features) == _features)
		{
			return format;
		}
	}

	return VK_FORMAT_UNDEFINED;
}
//======================================================================================================================
VkFormat FindDepthFormat(std::reference_wrapper<VkPhysicalDevice> _physicalDevice)
{
	return FindSupportedFormat(_physicalDevice,
							   {VK_FORMAT_D32_SFLOAT, VK_FORMAT_D32_SFLOAT_S8_UINT, VK_FORMAT_D24_UNORM_S8_UINT},
							   VK_IMAGE_TILING_OPTIMAL,
							   VK_FORMAT_FEATURE_DEPTH_STENCIL_ATTACHMENT_BIT);
}
//======================================================================================================================
bool HasStencilComponent(VkFormat _format)
{
	return _format == VK_FORMAT_D32_SFLOAT_S8_UINT || _format == VK_FORMAT_D24_UNORM_S8_UINT;
}

}