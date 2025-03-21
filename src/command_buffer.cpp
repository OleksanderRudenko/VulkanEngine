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
bool CommandBuffer::Create(std::shared_ptr<CommandPool> _commandPool)
{
	VkCommandBufferAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
	allocInfo.commandPool			= _commandPool->GetPool();
	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
	allocInfo.commandBufferCount	= 1;

	if (vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &buffer_) != VK_SUCCESS)
	{
		std::cout << "failed to allocate command buffer!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool CommandBuffer::CopyBuffer(VkBuffer		_src,
							   VkBuffer		_dst,
							   VkDeviceSize _size)
{
	if(!Begin())
	{
		return false;
	}
	VkBufferCopy copyRegion{};
	copyRegion.size = _size;
	vkCmdCopyBuffer(buffer_, _src, _dst, 1, &copyRegion);

	return End();
}
//======================================================================================================================
bool CommandBuffer::Begin(VkCommandBufferUsageFlags _usage)
{
	if(isRecording_.load() == true)
	{
		return true;
	}

	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;

	if(vkBeginCommandBuffer(buffer_, &beginInfo) != VK_SUCCESS)
	{
		std::cout << "failed to begin command buffer!\n";
		return false;
	}
	isRecording_.store(true);
	return true;
}
//======================================================================================================================
bool CommandBuffer::End()
{
	if(isRecording_.load() == false)
	{
		return true;
	}

	VkResult result = vkEndCommandBuffer(buffer_);
	if(result != VK_SUCCESS)
	{
		std::cout << "failed to end command buffer!\n";
		return false;
	}
	isRecording_.store(false);
	return true;
}
//======================================================================================================================
void CommandBuffer::SubmitAndWaitIdle(VkQueue _graphicsQueue)
{
	if(isRecording_.load() == true)
	{
		End();
	}
	;
	VkSubmitInfo submitInfo{};
	submitInfo.sType				= VK_STRUCTURE_TYPE_SUBMIT_INFO;
	submitInfo.commandBufferCount	= 1;
	submitInfo.pCommandBuffers		= &buffer_;

	vkQueueSubmit(_graphicsQueue, 1, &submitInfo, VK_NULL_HANDLE);
	vkQueueWaitIdle(_graphicsQueue);
}
//======================================================================================================================
//bool CommandBuffer::BeginSingleTimeCommand(CommandPool* _commandPool)
//{
//	VkCommandBufferAllocateInfo allocInfo{};
//	allocInfo.sType					= VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
//	allocInfo.level					= VK_COMMAND_BUFFER_LEVEL_PRIMARY;
//	allocInfo.commandPool			= _commandPool->GetPool();
//	allocInfo.commandBufferCount	= 1;
//
//	VkCommandBuffer commandBuffer;
//	vkAllocateCommandBuffers(logicalDevice_, &allocInfo, &commandBuffer);
//
//	VkCommandBufferBeginInfo beginInfo{};
//	beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
//	beginInfo.flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT;
//
//	vkBeginCommandBuffer(commandBuffer, &beginInfo);
//}

}