#pragma once

#include <vector>
#include <optional>
#include <fstream>
#include <vulkan/vulkan.h>

namespace xengine
{

struct QueueFamilyIndices
{
	std::optional<uint32_t> graphicsFamily;
	std::optional<uint32_t> presentFamily;

	bool isComplete()
	{
		return graphicsFamily.has_value() && presentFamily.has_value();
	}
};

std::vector<char>	ReadFile(const std::string& filename,
							 std::ios_base::openmode mode = std::ios::ate | std::ios::binary);

VkFormat			FindSupportedFormat(std::reference_wrapper<VkPhysicalDevice>,
										const std::vector<VkFormat>& candidates,
										VkImageTiling,
										VkFormatFeatureFlags);
VkFormat			FindDepthFormat(std::reference_wrapper<VkPhysicalDevice>);

bool				HasStencilComponent(VkFormat format);

}