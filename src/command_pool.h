#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>

namespace xengine
{

//Manages memory for command buffers
//One per thread
class ENGINE_API CommandPool final
{
public:
	CommandPool(VkDevice logicalDevice,
				VkPhysicalDevice physicalDevice,
				const QueueFamilyIndices&);
	CommandPool(const CommandPool&)					= delete;
	CommandPool(CommandPool&&)						= delete;
	~CommandPool();

	CommandPool&	operator=(const CommandPool&)	= delete;
	CommandPool&	operator=(CommandPool&&)		= delete;

	bool					Create();
	const VkCommandPool&	GetPool()	const		{ return commandPool_; }

private:
	VkDevice											logicalDevice_;
	VkPhysicalDevice									physicalDevice_;
	const QueueFamilyIndices						indices_;

	VkCommandPool									commandPool_ = VK_NULL_HANDLE;
};

}