#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>

namespace xengine
{

class ENGINE_API Buffer
{
public:
	Buffer(VkDeviceSize,
		   std::reference_wrapper<VkDevice> logicalDevice);
	Buffer(const Buffer&)				= delete;
	Buffer(Buffer&&)					= delete;
	virtual ~Buffer();

	Buffer&	operator=(const Buffer&)	= delete;
	Buffer&	operator=(Buffer&&)			= delete;

	bool					CreateBuffer(const VkPhysicalDevice&,
										 VkBufferUsageFlags,
										 VkMemoryPropertyFlags);
	static uint32_t			FindMemoryType(const VkPhysicalDevice&,
										   uint32_t typeFilter,
										   VkMemoryPropertyFlags);

	VkDeviceSize			GetSize()			const { return size_; }
	const VkBuffer&			GetBuffer()			const { return buffer_; }
	const VkDeviceMemory&	GetBufferMemory()	const { return bufferMemory_; }

protected:
	const std::reference_wrapper<VkDevice>	logicalDevice_;
	VkDeviceSize							size_			= 0;
	VkBuffer								buffer_			= VK_NULL_HANDLE;
	VkDeviceMemory							bufferMemory_	= VK_NULL_HANDLE;
};

}