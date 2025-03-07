#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace xengine
{

class CommandPool;

// Stores GPU commands
class ENGINE_API CommandBuffer final
{
public:
	CommandBuffer(std::reference_wrapper<VkDevice>			logicalDevice,
				  std::reference_wrapper<VkPhysicalDevice>	physicalDevice,
				  const QueueFamilyIndices&);
	CommandBuffer(const CommandBuffer&)				= delete;
	CommandBuffer(CommandBuffer&&)					= delete;
	~CommandBuffer();

	CommandBuffer&	operator=(const CommandBuffer&)	= delete;
	CommandBuffer&	operator=(CommandBuffer&&)		= delete;

	bool					CreateBuffer(CommandPool*);
	const VkCommandBuffer&	GetBuffer() const		{ return commandBuffer_; }

private:
	const std::reference_wrapper<VkDevice>			logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>	physicalDevice_;
	const QueueFamilyIndices						indices_;

	VkCommandBuffer									commandBuffer_ = VK_NULL_HANDLE;
};

}