#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace xengine
{

class CommandPool;
struct QueueFamilyIndices;

// Stores GPU commands
class ENGINE_API CommandBuffer final
{
public:
	CommandBuffer(VkDevice logicalDevice,
				  VkPhysicalDevice physicalDevice,
				  const QueueFamilyIndices&);
	CommandBuffer(const CommandBuffer&)				= delete;
	CommandBuffer(CommandBuffer&&)					= delete;
	~CommandBuffer()								= default;

	CommandBuffer&	operator=(const CommandBuffer&)	= delete;
	CommandBuffer&	operator=(CommandBuffer&&)		= delete;

	const VkCommandBuffer&	GetBuffer() const		{ return buffer_; }
	bool					Create(std::shared_ptr<CommandPool>);
	bool					CopyBuffer(VkBuffer		src,
									   VkBuffer		dst,
									   VkDeviceSize);
	bool					Begin(VkCommandBufferUsageFlags usage = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT);
	bool					End();
	void					SubmitAndWaitIdle(VkQueue graphicsQueue);
	//bool					BeginSingleTimeCommand(CommandPool*);

private:
	VkDevice											logicalDevice_;
	VkPhysicalDevice									physicalDevice_;
	const QueueFamilyIndices						indices_;

	VkCommandBuffer									buffer_			= VK_NULL_HANDLE;
	std::atomic<bool>								isRecording_	= {false};
};

}