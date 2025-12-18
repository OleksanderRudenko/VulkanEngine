#include "stdafx.h"
#include "pipeline.h"
#include "render_pass.h"
#include "command_pool.h"
#include "command_buffer.h"
#include "swapchain.h"
#include "tools.h"
#include "window.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Pipeline::Pipeline(VkDevice				_logicalDevice,
				   VkPhysicalDevice		_physicalDevice,
				   Swapchain*			_swapChain,
				   const QueueFamilyIndices&	_indices,
				   std::shared_ptr<Window>	_window,
				   ResourceManager*		_resourceManager,
				   ImGuiManager*		_imguiManager)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, swapChain_(_swapChain)
, indices_(_indices)
, window_(_window)
, resourceManager_(_resourceManager)
, imguiManager_(_imguiManager)
{}
//======================================================================================================================
Pipeline::~Pipeline()
{
	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		vkDestroySemaphore(logicalDevice_, imageAvailableSemaphores_[i], nullptr);
		vkDestroySemaphore(logicalDevice_, renderFinishedSemaphores_[i], nullptr);
		vkDestroyFence(logicalDevice_, inFlightFences_[i], nullptr);
	}

	commandBuffers_.clear();
	commandPool_.reset();
	renderPass_.reset();
}
//======================================================================================================================
bool Pipeline::Create()
{
	renderPass_ = std::make_shared<RenderPass>(logicalDevice_,
											   physicalDevice_,
											   swapChain_,
											   resourceManager_,
											   imguiManager_);
	if(!renderPass_->Create())
	{
		return false;
	}

	if(!CreateSyncObjects())
	{
		return false;
	}

	commandPool_ = std::make_shared<CommandPool>(logicalDevice_,
												 physicalDevice_,
												 indices_);
	commandPool_->Create();
	CreateCommandBuffers();
	return true;
}
//======================================================================================================================
void Pipeline::SetImGuiManager(ImGuiManager* _imguiManager)
{
	imguiManager_ = _imguiManager;
	if (renderPass_)
	{
		renderPass_->SetImGuiManager(_imguiManager);
	}
}
//======================================================================================================================
bool Pipeline::RenderFrame(const std::vector<std::shared_ptr<Sprite>>&	_sprites,
						   VkQueue										_graphicsQueue,
						   VkQueue										_presentQueue)
{
	vkWaitForFences(logicalDevice_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);
	vkResetFences(logicalDevice_, 1, &inFlightFences_[currentFrame_]);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice_,
											swapChain_->GetSwapChain(),
											UINT64_MAX,
											imageAvailableSemaphores_[currentFrame_],
											VK_NULL_HANDLE,
											&imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		swapChain_->Recreate(renderPass_->GetRenderPass());
		return true;  // Successfully handled, not an error
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		std::cout << "failed to acquire swap chain image!\n";
		return false;
	}

	vkResetCommandBuffer(commandBuffers_[currentFrame_]->GetBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
	if(!RecordCommandBuffer(commandBuffers_[currentFrame_]->GetBuffer(), imageIndex, _sprites))
	{
		return false;
	}

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]		= {imageAvailableSemaphores_[currentFrame_]};
	VkPipelineStageFlags waitStages[]	= {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= waitSemaphores;
	submitInfo.pWaitDstStageMask		= waitStages;
	submitInfo.commandBufferCount		= 1;
	submitInfo.pCommandBuffers			= &commandBuffers_[currentFrame_]->GetBuffer();

	VkSemaphore signalSemaphores[]		= {renderFinishedSemaphores_[currentFrame_]};
	submitInfo.signalSemaphoreCount		= 1;
	submitInfo.pSignalSemaphores		= signalSemaphores;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS)
	{
		std::cout << "failed to submit draw command buffer!\n";
		return false;
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= signalSemaphores;

	VkSwapchainKHR swapChains[]		= {swapChain_->GetSwapChain()};
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= swapChains;
	presentInfo.pImageIndices		= &imageIndex;
	presentInfo.pResults			= nullptr;

	result = vkQueuePresentKHR(_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_->FramebufferResized())
	{
		window_->FramebufferResizedReset();
		swapChain_->Recreate(renderPass_->GetRenderPass());
	}
	else if (result != VK_SUCCESS)
	{
		std::cout << "failed to present swap chain image!\n";
		return false;
	}

	currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
	return true;
}
//======================================================================================================================
bool Pipeline::CreateSyncObjects()
{
	imageAvailableSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	renderFinishedSemaphores_.resize(MAX_FRAMES_IN_FLIGHT);
	inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

	VkSemaphoreCreateInfo semaphoreInfo{};
	semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

	VkFenceCreateInfo fenceInfo{};
	fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
	fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

	for(size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++)
	{
		if (vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &imageAvailableSemaphores_[i]) != VK_SUCCESS
			|| vkCreateSemaphore(logicalDevice_, &semaphoreInfo, nullptr, &renderFinishedSemaphores_[i]) != VK_SUCCESS
			|| vkCreateFence(logicalDevice_, &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS)
		{
			std::cout << "failed to create semaphores!\n";
			return false;
		}
	}
	return true;
}
//======================================================================================================================
bool Pipeline::CreateCommandBuffers()
{
	commandBuffers_.reserve(MAX_FRAMES_IN_FLIGHT);

	for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; ++i)
	{
		commandBuffers_.emplace_back(std::make_unique<CommandBuffer>(logicalDevice_,
																	 physicalDevice_,
																	 indices_));
		if(!commandBuffers_[i]->Create(commandPool_))
		{
			return false;
		}
	}
	return true;
}
//======================================================================================================================
bool Pipeline::RecordCommandBuffer(VkCommandBuffer	_commandBuffer,
								   uint32_t			_imageIndex,
								   const std::vector<std::shared_ptr<Sprite>>& _sprites)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags				= 0; // Optional
	beginInfo.pInheritanceInfo	= nullptr; // Optional

	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		std::cout << "failed to begin recording command buffer!\n";
		return false;
	}

	return renderPass_->Render(_commandBuffer, _imageIndex, _sprites);
}

}