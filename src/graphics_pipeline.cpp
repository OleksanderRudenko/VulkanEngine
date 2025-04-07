#include "stdafx.h"
#include "graphics_pipeline.h"
#include "tools.h"
#include "swapchain.h"
#include "vertex.h"
#include <iostream>

namespace xengine
{

GraphicsPipeline::GraphicsPipeline(std::reference_wrapper<VkDevice> logicalDevice, std::reference_wrapper<Swapchain> _swapChain)
: logicalDevice_(logicalDevice)
, swapChain_(_swapChain)
{}
//======================================================================================================================
GraphicsPipeline::~GraphicsPipeline()
{
	Cleanup();
}
//======================================================================================================================
bool GraphicsPipeline::Create(VkRenderPass _renderPass)
{
	auto vertShaderCode = ReadFile("../src/shaders/vert.spv");
	auto fragShaderCode = ReadFile("../src/shaders/frag.spv");

	VkShaderModule vertShaderModule = CreateShaderModule(vertShaderCode);
	VkShaderModule fragShaderModule = CreateShaderModule(fragShaderCode);

	if(vertShaderModule == VK_NULL_HANDLE || fragShaderModule == VK_NULL_HANDLE)
	{
		return false;
	}

	VkPipelineShaderStageCreateInfo vertShaderStageInfo{};
	vertShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	vertShaderStageInfo.stage	= VK_SHADER_STAGE_VERTEX_BIT;
	vertShaderStageInfo.module	= vertShaderModule;
	vertShaderStageInfo.pName	= "main";

	VkPipelineShaderStageCreateInfo fragShaderStageInfo{};
	fragShaderStageInfo.sType	= VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
	fragShaderStageInfo.stage	= VK_SHADER_STAGE_FRAGMENT_BIT;
	fragShaderStageInfo.module	= fragShaderModule;
	fragShaderStageInfo.pName	= "main";

	VkPipelineShaderStageCreateInfo shaderStages[] = { vertShaderStageInfo, fragShaderStageInfo };

	auto bindingDescription		= Vertex::GetBindingDescription();
	auto attributeDescriptions	= Vertex::GetAttributeDescriptions();

	VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
	vertexInputInfo.sType							= VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
	vertexInputInfo.vertexBindingDescriptionCount	= 1;
	vertexInputInfo.vertexAttributeDescriptionCount	= static_cast<uint32_t>(attributeDescriptions.size());
	vertexInputInfo.pVertexBindingDescriptions		= &bindingDescription;
	vertexInputInfo.pVertexAttributeDescriptions	= attributeDescriptions.data();

	VkPipelineInputAssemblyStateCreateInfo inputAssembly{};
	inputAssembly.sType						= VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
	inputAssembly.topology					= VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
	inputAssembly.primitiveRestartEnable	= VK_FALSE;

	VkViewport viewport{};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= static_cast<float>(swapChain_.get().GetSwapChainExtent().width);
	viewport.height		= static_cast<float>(swapChain_.get().GetSwapChainExtent().height);
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;

	VkRect2D scissor{};
	scissor.offset = { 0, 0 };
	scissor.extent = swapChain_.get().GetSwapChainExtent();

	VkPipelineViewportStateCreateInfo viewportState{};
	viewportState.sType				= VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
	viewportState.viewportCount		= 1;
	viewportState.pViewports		= &viewport;
	viewportState.scissorCount		= 1;
	viewportState.pScissors			= &scissor;

	VkPipelineRasterizationStateCreateInfo rasterizer{};
	rasterizer.sType					= VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
	rasterizer.depthClampEnable			= VK_FALSE;
	rasterizer.rasterizerDiscardEnable	= VK_FALSE;
	rasterizer.polygonMode				= VK_POLYGON_MODE_FILL;
	rasterizer.lineWidth				= 1.0f;
	rasterizer.cullMode					= VK_CULL_MODE_BACK_BIT;
	rasterizer.frontFace				= VK_FRONT_FACE_COUNTER_CLOCKWISE;
	rasterizer.depthBiasEnable			= VK_FALSE;

	VkPipelineMultisampleStateCreateInfo multisampling{};
	multisampling.sType					= VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
	multisampling.sampleShadingEnable	= VK_FALSE;
	multisampling.rasterizationSamples	= VK_SAMPLE_COUNT_1_BIT;

	VkPipelineColorBlendAttachmentState colorBlendAttachment{};
	colorBlendAttachment.colorWriteMask			= VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;
	colorBlendAttachment.blendEnable			= VK_TRUE;
	colorBlendAttachment.srcColorBlendFactor	= VK_BLEND_FACTOR_SRC_ALPHA;
	colorBlendAttachment.dstColorBlendFactor	= VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
	colorBlendAttachment.colorBlendOp			= VK_BLEND_OP_ADD;
	colorBlendAttachment.srcAlphaBlendFactor	= VK_BLEND_FACTOR_ONE;
	colorBlendAttachment.dstAlphaBlendFactor	= VK_BLEND_FACTOR_ZERO;
	colorBlendAttachment.alphaBlendOp			= VK_BLEND_OP_ADD;

	VkPipelineColorBlendStateCreateInfo colorBlending{};
	colorBlending.sType				= VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
	colorBlending.logicOpEnable		= VK_FALSE;
	colorBlending.logicOp			= VK_LOGIC_OP_COPY;
	colorBlending.attachmentCount	= 1;
	colorBlending.pAttachments		= &colorBlendAttachment;

	std::vector<VkDynamicState> dynamicStates =
	{
		VK_DYNAMIC_STATE_VIEWPORT,
		VK_DYNAMIC_STATE_SCISSOR
	};

	VkPipelineDynamicStateCreateInfo dynamicState{};
	dynamicState.sType				= VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
	dynamicState.dynamicStateCount	= static_cast<uint32_t>(dynamicStates.size());
	dynamicState.pDynamicStates		= dynamicStates.data();


	VkDescriptorSetLayoutBinding uboLayoutBinding{};
	uboLayoutBinding.binding			= 0;
	uboLayoutBinding.descriptorType		= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	uboLayoutBinding.descriptorCount	= 1;
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;
	uboLayoutBinding.pImmutableSamplers	= nullptr; // Optional
	uboLayoutBinding.stageFlags			= VK_SHADER_STAGE_VERTEX_BIT;

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
		std::cout << "failed to create descriptor set layout!!\n";
		return false;
	}

	VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
	pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.sType					= VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
	pipelineLayoutInfo.setLayoutCount			= 1; // Optional
	pipelineLayoutInfo.pSetLayouts				= &descriptorSetLayout_; // Optional
	pipelineLayoutInfo.pushConstantRangeCount	= 0; // Optional
	pipelineLayoutInfo.pPushConstantRanges		= nullptr; // Optional

	if (vkCreatePipelineLayout(logicalDevice_.get(), &pipelineLayoutInfo, nullptr, &pipelineLayout_) != VK_SUCCESS)
	{
		std::cout << "failed to create pipeline layout!\n";
		return false;
	}

	VkPipelineDepthStencilStateCreateInfo depthStencil{};
	depthStencil.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
	depthStencil.depthTestEnable = VK_TRUE;
	depthStencil.depthWriteEnable = VK_TRUE;
	depthStencil.depthCompareOp = VK_COMPARE_OP_LESS;
	depthStencil.depthBoundsTestEnable = VK_FALSE;
	depthStencil.stencilTestEnable = VK_FALSE;

	VkGraphicsPipelineCreateInfo pipelineInfo{};
	pipelineInfo.sType					= VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
	pipelineInfo.stageCount				= 2;
	pipelineInfo.pStages				= shaderStages;
	pipelineInfo.pVertexInputState		= &vertexInputInfo;
	pipelineInfo.pInputAssemblyState	= &inputAssembly;
	pipelineInfo.pViewportState			= &viewportState;
	pipelineInfo.pRasterizationState	= &rasterizer;
	pipelineInfo.pMultisampleState		= &multisampling;
	pipelineInfo.pColorBlendState		= &colorBlending;
	pipelineInfo.pDepthStencilState		= &depthStencil;
	pipelineInfo.pDynamicState			= &dynamicState;
	pipelineInfo.layout					= pipelineLayout_;
	pipelineInfo.renderPass				= _renderPass;
	pipelineInfo.subpass				= 0;

	if (vkCreateGraphicsPipelines(logicalDevice_.get(), VK_NULL_HANDLE, 1, &pipelineInfo, nullptr, &graphicsPipeline_) != VK_SUCCESS)
	{
		std::cout << "failed to create graphics pipeline!\n";
		return false;
	}

	vkDestroyShaderModule(logicalDevice_.get(), fragShaderModule, nullptr);
	vkDestroyShaderModule(logicalDevice_.get(), vertShaderModule, nullptr);

	return true;
}
//======================================================================================================================
void GraphicsPipeline::Cleanup()
{
	vkDestroyPipeline(logicalDevice_.get(), graphicsPipeline_, nullptr);
	vkDestroyPipelineLayout(logicalDevice_.get(), pipelineLayout_, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice_.get(), descriptorSetLayout_, nullptr);
}
//======================================================================================================================
VkShaderModule GraphicsPipeline::CreateShaderModule(const std::vector<char>& code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize = code.size();
	createInfo.pCode = reinterpret_cast<const uint32_t*>(code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		std::cout << "failed to create shader module!\n";
		return VK_NULL_HANDLE;
	}

	return shaderModule;
}

}
