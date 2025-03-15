#pragma once

#include <vector>
#include <optional>
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

}