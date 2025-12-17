#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>

namespace xengine
{

class Instance;
class Surface;

class ENGINE_API DeviceManager
{
public:
	DeviceManager(Instance* instance,
				  Surface* surface);
	DeviceManager(const DeviceManager&)								= delete;
	DeviceManager(DeviceManager&&)									= delete;
	~DeviceManager();

	DeviceManager&				operator=(const DeviceManager&)		= delete;
	DeviceManager&				operator=(DeviceManager&&)			= delete;

	bool						Create();

	VkPhysicalDevice			GetPhysicalDevice()			const	{ return physicalDevice_; }
	VkDevice					GetLogicalDevice()			const	{ return logicalDevice_; }
	VkQueue						GetGraphicsQueue()			const	{ return graphicsQueue_; }
	VkQueue						GetPresentQueue()			const	{ return presentQueue_; }
	const QueueFamilyIndices&	GetQueueFamilyIndices()		const	{ return indices_; }

private:
	bool						PickPhysicalDevice();
	bool						CreateLogicalDevice();
	bool						IsDeviceSuitable(VkPhysicalDevice device);
	bool						CheckDeviceExtensionSupport(VkPhysicalDevice device);

	Instance*			instance_;
	Surface*			surface_;

	VkPhysicalDevice	physicalDevice_	= VK_NULL_HANDLE;
	VkDevice			logicalDevice_	= VK_NULL_HANDLE;
	VkQueue				graphicsQueue_	= VK_NULL_HANDLE;
	VkQueue				presentQueue_	= VK_NULL_HANDLE;
	QueueFamilyIndices	indices_;
};

}
