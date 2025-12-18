#include "stdafx.h"
#include "imgui_manager.h"
#include <imgui.h>
#include <imgui_impl_glfw.h>
#include <imgui_impl_vulkan.h>
#include <iostream>

namespace xengine
{

//======================================================================================================================
ImGuiManager::ImGuiManager(VkInstance			_instance,
						   VkDevice				_logicalDevice,
						   VkPhysicalDevice		_physicalDevice,
						   uint32_t				_queueFamily,
						   VkQueue				_graphicsQueue,
						   VkRenderPass			_renderPass,
						   uint32_t				_imageCount)
: instance_(_instance)
, logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, queueFamily_(_queueFamily)
, graphicsQueue_(_graphicsQueue)
, renderPass_(_renderPass)
, imageCount_(_imageCount)
{}
//======================================================================================================================
ImGuiManager::~ImGuiManager()
{
	Shutdown();
}
//======================================================================================================================
bool ImGuiManager::Init(GLFWwindow* _window)
{
	// Create descriptor pool for ImGui
	VkDescriptorPoolSize poolSizes[] =
	{
		{ VK_DESCRIPTOR_TYPE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1000 },
		{ VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_IMAGE, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_TEXEL_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 1000 },
		{ VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC, 1000 },
		{ VK_DESCRIPTOR_TYPE_INPUT_ATTACHMENT, 1000 }
	};

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.flags			= VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
	poolInfo.maxSets		= 1000;
	poolInfo.poolSizeCount	= std::size(poolSizes);
	poolInfo.pPoolSizes		= poolSizes;

	if (vkCreateDescriptorPool(logicalDevice_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
	{
		std::cout << "failed to create ImGui descriptor pool!\n";
		return false;
	}

	// Setup Dear ImGui context
	IMGUI_CHECKVERSION();
	ImGui::CreateContext();
	ImGuiIO& io = ImGui::GetIO();
	io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;	// Enable Keyboard Controls

	// Setup Dear ImGui style
	ImGui::StyleColorsDark();

	// Setup Platform/Renderer backends
	ImGui_ImplGlfw_InitForVulkan(_window, true);

	ImGui_ImplVulkan_InitInfo initInfo{};
	initInfo.Instance		= instance_;
	initInfo.PhysicalDevice	= physicalDevice_;
	initInfo.Device			= logicalDevice_;
	initInfo.QueueFamily	= queueFamily_;
	initInfo.Queue			= graphicsQueue_;
	initInfo.DescriptorPool	= descriptorPool_;
	initInfo.MinImageCount	= imageCount_;
	initInfo.ImageCount		= imageCount_;

	// Setup pipeline info for the new ImGui API
	initInfo.PipelineInfoMain.RenderPass	= renderPass_;
	initInfo.PipelineInfoMain.Subpass		= 0;
	initInfo.PipelineInfoMain.MSAASamples	= VK_SAMPLE_COUNT_1_BIT;

	if (!ImGui_ImplVulkan_Init(&initInfo))
	{
		std::cout << "failed to initialize ImGui Vulkan backend!\n";
		return false;
	}

	initialized_ = true;
	return true;
}
//======================================================================================================================
void ImGuiManager::NewFrame()
{
	if (!initialized_)
		return;

	ImGui_ImplVulkan_NewFrame();
	ImGui_ImplGlfw_NewFrame();
	ImGui::NewFrame();
}
//======================================================================================================================
void ImGuiManager::Render(VkCommandBuffer _commandBuffer)
{
	if (!initialized_)
		return;

	ImGui::Render();
	ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), _commandBuffer);
}
//======================================================================================================================
void ImGuiManager::Shutdown()
{
	if (!initialized_)
		return;

	vkDeviceWaitIdle(logicalDevice_);

	ImGui_ImplVulkan_Shutdown();
	ImGui_ImplGlfw_Shutdown();
	ImGui::DestroyContext();

	if (descriptorPool_ != VK_NULL_HANDLE)
	{
		vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
		descriptorPool_ = VK_NULL_HANDLE;
	}

	initialized_ = false;
}

}
