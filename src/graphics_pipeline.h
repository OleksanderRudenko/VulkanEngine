#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <vector>

namespace xengine
{

class Swapchain;

class GraphicsPipeline
{
public:
	GraphicsPipeline(VkDevice logicalDevice,
					 Swapchain* swapchain);
	GraphicsPipeline(const GraphicsPipeline&)				= delete;
	GraphicsPipeline(GraphicsPipeline&&)					= delete;
	~GraphicsPipeline();

	GraphicsPipeline&	operator=(const GraphicsPipeline&)	= delete;
	GraphicsPipeline&	operator=(GraphicsPipeline&&)		= delete;

	bool				Create(VkRenderPass);
	void				Cleanup();

	VkPipeline			GetPipeline()		const { return graphicsPipeline_; }
	VkPipelineLayout	GetPipelineLayout()	const { return pipelineLayout_; }

private:
	VkShaderModule		CreateShaderModule(const std::vector<char>& code);

	VkDevice								logicalDevice_;
	Swapchain*								swapChain_;

	VkPipeline								graphicsPipeline_		= VK_NULL_HANDLE;
	VkPipelineLayout						pipelineLayout_			= VK_NULL_HANDLE;
	VkDescriptorSetLayout					descriptorSetLayout_	= VK_NULL_HANDLE;
};

}