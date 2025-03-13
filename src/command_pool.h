#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>

namespace xengine
{

//Manages memory for command buffers
class ENGINE_API CommandPool final
{
public:
	CommandPool(std::reference_wrapper<VkDevice>			logicalDevice,
				std::reference_wrapper<VkPhysicalDevice>	physicalDevice,
				const QueueFamilyIndices&);
	CommandPool(const CommandPool&)					= delete;
	CommandPool(CommandPool&&)						= delete;
	~CommandPool();

	CommandPool&	operator=(const CommandPool&)	= delete;
	CommandPool&	operator=(CommandPool&&)		= delete;

	bool					Create();
	const VkCommandPool&	GetPool()	const		{ return commandPool_; }

private:
	const std::reference_wrapper<VkDevice>			logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>	physicalDevice_;
	const QueueFamilyIndices						indices_;

	VkCommandPool									commandPool_ = VK_NULL_HANDLE;
};

}