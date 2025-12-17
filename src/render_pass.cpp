#include "stdafx.h"
#include "render_pass.h"
#include "buffer.h"
#include "graphics_pipeline.h"
#include "resource_manager.h"
#include "sprite.h"
#include "swapchain.h"
#include "vertex.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
RenderPass::RenderPass(VkDevice			_logicalDevice,
					   VkPhysicalDevice	_physicalDevice,
					   Swapchain*		_swapChain,
					   ResourceManager*	_resourceManager)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, swapChain_(_swapChain)
, resourceManager_(_resourceManager)
{}
//======================================================================================================================
RenderPass::~RenderPass()
{
	Cleanup();
}
//======================================================================================================================
bool RenderPass::Create()
{
	VkAttachmentDescription colorAttachment{};
	colorAttachment.format			= swapChain_->GetSwapChainImageFormat();
	colorAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
	colorAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	colorAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_STORE;
	colorAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	colorAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	colorAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	colorAttachment.finalLayout		= VK_IMAGE_LAYOUT_PRESENT_SRC_KHR;

	VkAttachmentDescription depthAttachment{};
	depthAttachment.format			= FindDepthFormat(physicalDevice_);
	depthAttachment.samples			= VK_SAMPLE_COUNT_1_BIT;
	depthAttachment.loadOp			= VK_ATTACHMENT_LOAD_OP_CLEAR;
	depthAttachment.storeOp			= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.stencilLoadOp	= VK_ATTACHMENT_LOAD_OP_DONT_CARE;
	depthAttachment.stencilStoreOp	= VK_ATTACHMENT_STORE_OP_DONT_CARE;
	depthAttachment.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	depthAttachment.finalLayout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkAttachmentReference colorAttachmentRef{};
	colorAttachmentRef.attachment	= 0;
	colorAttachmentRef.layout		= VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;

	VkAttachmentReference depthAttachmentRef{};
	depthAttachmentRef.attachment	= 1;
	depthAttachmentRef.layout		= VK_IMAGE_LAYOUT_DEPTH_STENCIL_ATTACHMENT_OPTIMAL;

	VkSubpassDescription subpass{};
	subpass.pipelineBindPoint		= VK_PIPELINE_BIND_POINT_GRAPHICS;
	subpass.colorAttachmentCount	= 1;
	subpass.pColorAttachments		= &colorAttachmentRef;
	subpass.pDepthStencilAttachment = &depthAttachmentRef;

	VkSubpassDependency dependency{};
	dependency.srcSubpass			= VK_SUBPASS_EXTERNAL;
	dependency.dstSubpass			= 0;
	dependency.srcStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.srcAccessMask		= 0;
	dependency.dstStageMask			= VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT | VK_PIPELINE_STAGE_EARLY_FRAGMENT_TESTS_BIT;
	dependency.dstAccessMask		= VK_ACCESS_COLOR_ATTACHMENT_WRITE_BIT | VK_ACCESS_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT;

	std::array<VkAttachmentDescription, 2> attachments = {colorAttachment, depthAttachment};
	VkRenderPassCreateInfo renderPassInfo{};
	renderPassInfo.sType			= VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO;
	renderPassInfo.attachmentCount	= static_cast<uint32_t>(attachments.size());
	renderPassInfo.pAttachments		= attachments.data();
	renderPassInfo.subpassCount		= 1;
	renderPassInfo.pSubpasses		= &subpass;
	
	if (vkCreateRenderPass(logicalDevice_, &renderPassInfo, nullptr, &renderPass_) != VK_SUCCESS)
	{
		std::cout << "failed to create render pass!\n";
		return false;
	}

	// Create graphics pipeline
	graphicsPipeline_ = std::make_unique<GraphicsPipeline>(logicalDevice_,
														   swapChain_);

	return graphicsPipeline_->Create(renderPass_,
									 resourceManager_->GetDescriptorSetLayout(),
									 resourceManager_->GetPipelineLayout());
}
//======================================================================================================================
bool RenderPass::Render(VkCommandBuffer								_commandBuffer,
						uint32_t									_imageIndex,
						const std::vector<std::shared_ptr<Sprite>>&	_sprites)
{
	VkRenderPassBeginInfo renderPassInfo{};
	renderPassInfo.sType				= VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
	renderPassInfo.renderPass			= renderPass_;
	renderPassInfo.framebuffer			= swapChain_->GetSwapChainFramebuffers()[_imageIndex];
	renderPassInfo.renderArea.offset	= { 0, 0 };
	renderPassInfo.renderArea.extent	= swapChain_->GetSwapChainExtent();

	// VkClearValue ? move into pipeline
	std::array<VkClearValue, 2> clearValues{};
	clearValues[0].color = { { 0.0f, 0.0f, 0.0f, 0.0f } };
	clearValues[1].depthStencil = { 1.0f, 0 }; //clearValues[0].depthStencil is ignored
	renderPassInfo.clearValueCount	= 2;
	renderPassInfo.pClearValues		= clearValues.data();

	vkCmdBeginRenderPass(_commandBuffer, &renderPassInfo, VK_SUBPASS_CONTENTS_INLINE);
	vkCmdBindPipeline(_commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline_->GetPipeline());

	VkViewport viewport{};
	viewport.x			= 0.0f;
	viewport.y			= 0.0f;
	viewport.width		= static_cast<float>(swapChain_->GetSwapChainExtent().width);
	viewport.height		= static_cast<float>(swapChain_->GetSwapChainExtent().height);
	viewport.minDepth	= 0.0f;
	viewport.maxDepth	= 1.0f;
	vkCmdSetViewport(_commandBuffer, 0, 1, &viewport);

	VkRect2D scissor{};
	scissor.offset = {0, 0};
	scissor.extent = swapChain_->GetSwapChainExtent();
	vkCmdSetScissor(_commandBuffer, 0, 1, &scissor);

	for (const auto& sprite : _sprites)
	{
		sprite->UpdateUbo(swapChain_->GetSwapChainExtent());
	}

	for (const auto& sprite : _sprites)
	{
		VkBuffer vertexBuffers[]	= { sprite->GetVertexBuffer()->GetBuffer()};
		VkDeviceSize offsets[]		= { 0 };
		vkCmdBindVertexBuffers(_commandBuffer, 0, 1, vertexBuffers, offsets);
		vkCmdBindIndexBuffer(_commandBuffer, sprite->GetIndexBuffer()->GetBuffer(), 0, VK_INDEX_TYPE_UINT16);
		vkCmdBindDescriptorSets(_commandBuffer,
								VK_PIPELINE_BIND_POINT_GRAPHICS,
								resourceManager_->GetPipelineLayout(),
								0,
								1,
								&sprite->GetDescriptorSet(),
								0,
								nullptr);
		//sprite->UpdateUbo();
		vkCmdDrawIndexed(_commandBuffer, static_cast<uint32_t>(indices.size()), 1, 0, 0, 0);
	}

	vkCmdEndRenderPass(_commandBuffer);
	if (vkEndCommandBuffer(_commandBuffer) != VK_SUCCESS)
	{
		std::cout << "failed to record command buffer!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
void RenderPass::Cleanup()
{
	graphicsPipeline_.reset();
	vkDestroyRenderPass(logicalDevice_, renderPass_, nullptr);
}

}