#include "command_pool.h"
#include "iostream"

namespace xengine
{

//======================================================================================================================
CommandPool::CommandPool(std::reference_wrapper<VkDevice>			_logicalDevice,
						 std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
						 const QueueFamilyIndices&					_indices)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, indices_(_indices)
{}
//======================================================================================================================
CommandPool::~CommandPool()
{
	//all VkCommandBuffer instances in it are freed automatically
	vkDestroyCommandPool(logicalDevice_, commandPool_, nullptr);
}
//======================================================================================================================
bool CommandPool::Create()
{
	VkCommandPoolCreateInfo poolInfo{};
	poolInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO;
	poolInfo.flags				= VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT;
	poolInfo.queueFamilyIndex	= indices_.graphicsFamily.value();
	if (vkCreateCommandPool(logicalDevice_, &poolInfo, nullptr, &commandPool_) != VK_SUCCESS)
	{
		std::cout << "failed to create command pool!\n";
		return false;
	}
	return true;
}

}