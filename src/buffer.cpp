#include "stdafx.h"
#include "buffer.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Buffer::Buffer(VkDeviceSize						_size,
			   std::reference_wrapper<VkDevice>	_logicalDevice)
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

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	= memRequirements.size;
	allocInfo.memoryTypeIndex	= FindMemoryType(_physicalDevice, memRequirements.memoryTypeBits, _properties);

	if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &bufferMemory_) != VK_SUCCESS)
	{
		std::cout << "failed to allocate buffer memory!\n";
		throw std::runtime_error("failed to allocate buffer memory!");
	}

	vkBindBufferMemory(logicalDevice_, buffer_, bufferMemory_, 0);
	return true;
}
//======================================================================================================================
uint32_t Buffer::FindMemoryType(const VkPhysicalDevice&	_physicalDevice,
								uint32_t				_typeFilter,
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

	throw std::runtime_error("failed to find suitable memory type!");
}

}