#include "stdafx.h"
#include "command_buffer.h"
#include "command_pool.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
CommandBuffer::CommandBuffer(std::reference_wrapper<VkDevice>			_logicalDevice,
							 std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
							 const QueueFamilyIndices&					_indices)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, indices_(_indices)
{}
//======================================================================================================================
CommandBuffer::~CommandBuffer()
{
	
}
//======================================================================================================================
bool CommandBuffer::CreateBuffer(CommandPool* _commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool			= _commandPool->GetPool();
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= 1;

	if (vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &commandBuffer_) != VK_SUCCESS)
	{
		std::cout << "failed to allocate command buffers!\n";
		return false;
	}
	return true;
}

}