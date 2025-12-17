#include "stdafx.h"
#include "buffer.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Buffer::Buffer(VkDeviceSize	_size,
			   VkDevice		_logicalDevice)
: size_(_size)
, logicalDevice_(_logicalDevice)
{}
//======================================================================================================================
Buffer::~Buffer()
{
	vkDestroyBuffer(logicalDevice_, buffer_, nullptr);
	vkFreeMemory(logicalDevice_, bufferMemory_, nullptr);
}
//======================================================================================================================
bool Buffer::CreateBuffer(const VkPhysicalDevice&	_physicalDevice,
						  VkBufferUsageFlags		_usage,
						  VkMemoryPropertyFlags		_properties)
{
	VkBufferCreateInfo bufferInfo{};
	bufferInfo.sType		= VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO;
	bufferInfo.size			= size_;
	bufferInfo.usage		= _usage;
	bufferInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateBuffer(logicalDevice_, &bufferInfo, nullptr, &buffer_) != VK_SUCCESS)
	{
		std::cout << "failed to create buffer!\n";
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetBufferMemoryRequirements(logicalDevice_, buffer_, &memRequirements);

	std::optional<uint32_t> memoryType = FindMemoryType(_physicalDevice, memRequirements.memoryTypeBits, _properties);
	if(!memoryType.has_value())
	{
		std::cout << "failed to find suitable memory type!\n";
		return false;
	}

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	= memRequirements.size;
	allocInfo.memoryTypeIndex	= memoryType.value();

	if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &bufferMemory_) != VK_SUCCESS)
	{
		std::cout << "failed to allocate buffer memory!\n";
		return false;
	}

	vkBindBufferMemory(logicalDevice_, buffer_, bufferMemory_, 0);
	return true;
}
//======================================================================================================================
std::optional<uint32_t> Buffer::FindMemoryType(const VkPhysicalDevice&	_physicalDevice,
											   uint32_t					_typeFilter,
											   VkMemoryPropertyFlags	_properties)
{
	VkPhysicalDeviceMemoryProperties memProperties;
	vkGetPhysicalDeviceMemoryProperties(_physicalDevice, &memProperties);
	for (uint32_t i = 0; i < memProperties.memoryTypeCount; i++)
	{
		if ((_typeFilter & (1 << i)) && (memProperties.memoryTypes[i].propertyFlags & _properties) == _properties)
		{
			return i;
		}
	}

	return {};
}

}