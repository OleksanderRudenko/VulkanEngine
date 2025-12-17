#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>

namespace xengine
{

class ENGINE_API ResourceManager
{
public:
	ResourceManager(VkDevice logicalDevice);
	ResourceManager(const ResourceManager&)				= delete;
	ResourceManager(ResourceManager&&)					= delete;
	~ResourceManager();

	ResourceManager&	operator=(const ResourceManager&)	= delete;
	ResourceManager&	operator=(ResourceManager&&)		= delete;

	// Initialization
	bool							Create();

	// Accessors
	VkDescriptorSetLayout			GetDescriptorSetLayout()	const	{ return descriptorSetLayout_; }
	VkPipelineLayout				GetPipelineLayout()			const	{ return pipelineLayout_; }

private:
	bool	CreateDescriptorSetLayout();
	bool	CreatePipelineLayout();

	VkDevice				logicalDevice_;
	VkDescriptorSetLayout	descriptorSetLayout_	= VK_NULL_HANDLE;
	VkPipelineLayout		pipelineLayout_			= VK_NULL_HANDLE;
};

}
