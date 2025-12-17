#include "stdafx.h"
#include "resource_manager.h"
#include <iostream>
#include <array>

namespace xengine
{

//======================================================================================================================
ResourceManager::ResourceManager(VkDevice _logicalDevice)
: logicalDevice_(_logicalDevice)
{}
//======================================================================================================================
ResourceManager::~ResourceManager()
{
	if (descriptorSetLayout_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorSetLayout(logicalDevice_, descriptorSetLayout_, nullptr);
	}
	if (pipelineLayout_ != VK_NULL_HANDLE)
	{
		vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);
	}
}
//======================================================================================================================
bool ResourceManager::Create()
{
	if (!CreateDescriptorSetLayout())
	{
		return false;
	}
	if (!CreatePipelineLayout())
	{
		return false;
	}
	return true;
}
//======================================================================================================================
bool ResourceManager::CreateDescriptorSetLayout()
{
	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding			= 0;
	uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount	= 1;
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers	= nullptr; // Optional

	VkDescriptorSetLayoutBinding samplerLayoutBinding{};
	samplerLayoutBinding.binding			= 1;
	samplerLayoutBinding.descriptorCount	= 1;
	samplerLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	samplerLayoutBinding.pImmutableSamplers	= nullptr;
	samplerLayoutBinding.stageFlags			= VK_SHADER_STAGE_FRAGMENT_BIT;

	std::array<VkDescriptorSetLayoutBinding, 2> bindings = {uboLayoutBinding, samplerLayoutBinding};
	VkDescriptorSetLayoutCreateInfo layoutInfo{};
	layoutInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO;
	layoutInfo.bindingCount		= static_cast<uint32_t>(bindings.size());
	layoutInfo.pBindings		= bindings.data();

	if (vkCreateDescriptorSetLayout(logicalDevice_, &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS)
	{
		std::cout << "failed to create descriptor set layout!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool ResourceManager::CreatePipelineLayout()
{
	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount			= 1;
	pipelineLayoutInfo.pSetLayouts				= &descriptorSetLayout_;
	pipelineLayoutInfo.pushConstantRangeCount	= 0;
	pipelineLayoutInfo.pPushConstantRanges		= nullptr;

	if (vkCreatePipelineLayout(logicalDevice_, &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
	{
		std::cout << "failed to create pipeline layout!\n";
		return false;
	}
	return true;
}

}
