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

static constexpr uint32_t MAX_FRAMES_IN_FLIGHT = 2;

//======================================================================================================================
Pipeline::Pipeline(std::reference_wrapper<VkDevice>				_logicalDevice,
				   std::reference_wrapper<VkPhysicalDevice>		_physicalDevice,
				   std::reference_wrapper<Swapchain>			_swapChain,
				   std::reference_wrapper<QueueFamilyIndices>	_indices,
				   std::shared_ptr<Window>						_window)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, swapChain_(_swapChain)
, indices_(_indices)
, window_(_window)
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
	renderPass_ = std::make_shared<RenderPass>(logicalDevice_, swapChain_);
	if(!renderPass_.get()->Create())
	{
		return false;
	}

	if(!CreateSyncObjects())
	{
		return false;
	}

	commandPool_ = std::make_shared<CommandPool>(std::ref(logicalDevice_),
												 std::ref(physicalDevice_),
												 indices_);
	commandPool_.get()->Create();
	CreateCommandBuffers();
	return true;
}
//======================================================================================================================
void Pipeline::RenderFrame(const std::vector<std::shared_ptr<Sprite>>&	_sprites,
						   VkQueue										_graphicsQueue,
						   VkQueue										_presentQueue)
{
	vkWaitForFences(logicalDevice_, 1, &inFlightFences_[currentFrame_], VK_TRUE, UINT64_MAX);

	uint32_t imageIndex;
	VkResult result = vkAcquireNextImageKHR(logicalDevice_,
											swapChain_.get().GetSwapChain(),
											UINT64_MAX,
											imageAvailableSemaphores_[currentFrame_],
											VK_NULL_HANDLE,
											&imageIndex);
	if (result == VK_ERROR_OUT_OF_DATE_KHR)
	{
		swapChain_.get().Recreate(renderPass_.get()->GetRenderPass());
		return;
	}
	else if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
	{
		// todo: return false instead of exception
		throw std::runtime_error("failed to acquire swap chain image!");
	}

	//for (const auto& sprite : _sprites)
	//{
	//	sprite->UpdateUbo();
	//}

	vkResetFences(logicalDevice_, 1, &inFlightFences_[currentFrame_]);
	vkResetCommandBuffer(commandBuffers_[currentFrame_].get()->GetBuffer(), /*VkCommandBufferResetFlagBits*/ 0);
	RecordCommandBuffer(commandBuffers_[currentFrame_].get()->GetBuffer(), imageIndex, _sprites);

	VkSubmitInfo submitInfo{};
	submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

	VkSemaphore waitSemaphores[]		= {imageAvailableSemaphores_[currentFrame_]};
	VkPipelineStageFlags waitStages[]	= {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
	submitInfo.waitSemaphoreCount		= 1;
	submitInfo.pWaitSemaphores			= waitSemaphores;
	submitInfo.pWaitDstStageMask		= waitStages;
	submitInfo.commandBufferCount		= 1;
	submitInfo.pCommandBuffers			= &commandBuffers_[currentFrame_].get()->GetBuffer();

	VkSemaphore signalSemaphores[]		= {renderFinishedSemaphores_[currentFrame_]};
	submitInfo.signalSemaphoreCount		= 1;
	submitInfo.pSignalSemaphores		= signalSemaphores;

	if (vkQueueSubmit(_graphicsQueue, 1, &submitInfo, inFlightFences_[currentFrame_]) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to submit draw command buffer!");
	}

	VkPresentInfoKHR presentInfo{};
	presentInfo.sType				= VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
	presentInfo.waitSemaphoreCount	= 1;
	presentInfo.pWaitSemaphores		= signalSemaphores;

	VkSwapchainKHR swapChains[]		= {swapChain_.get().GetSwapChain()};
	presentInfo.swapchainCount		= 1;
	presentInfo.pSwapchains			= swapChains;
	presentInfo.pImageIndices		= &imageIndex;
	presentInfo.pResults			= nullptr;

	result = vkQueuePresentKHR(_presentQueue, &presentInfo);
	if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || window_.get()->FramebufferResized())
	{
		window_.get()->FramebufferResizedReset();
		swapChain_.get().Recreate(renderPass_.get()->GetRenderPass());
	}
	else if (result != VK_SUCCESS)
	{
		throw std::runtime_error("failed to present swap chain image!");
	}

	currentFrame_ = (currentFrame_ + 1) % MAX_FRAMES_IN_FLIGHT;
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
		commandBuffers_.emplace_back(std::make_unique<CommandBuffer>(std::ref(logicalDevice_),
																	 std::ref(physicalDevice_),
																	 indices_));
		if(!commandBuffers_[i].get()->Create(commandPool_))
		{
			return false;
		}
	}
	return true;
}
//======================================================================================================================
void Pipeline::RecordCommandBuffer(VkCommandBuffer	_commandBuffer,
								   uint32_t			_imageIndex,
								   const std::vector<std::shared_ptr<Sprite>>& _sprites)
{
	VkCommandBufferBeginInfo beginInfo{};
	beginInfo.sType				= VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;
	beginInfo.flags				= 0; // Optional
	beginInfo.pInheritanceInfo	= nullptr; // Optional

	if (vkBeginCommandBuffer(_commandBuffer, &beginInfo) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to begin recording command buffer!");
	}

	renderPass_.get()->Render(_commandBuffer, _imageIndex, _sprites);
}

}