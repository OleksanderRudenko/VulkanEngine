#include "stdafx.h"
#include "application.h"
#include "vertex.h"
#include "uniform.h"
#include "tools/timer.h"
#include <algorithm>

namespace xengine
{

//======================================================================================================================
Application::Application(uint32_t _width,
						 uint32_t _height)
: window_(std::make_shared<Window>(_width, _height, "Vulkan Engine"))
{}
//======================================================================================================================
bool Application::Init()
{
	if(!window_->Init())
	{
		return false;
	}

	return InitVulkan();
}
//======================================================================================================================
void Application::Run()
{
	MainLoop();
	Cleanup();
}
//======================================================================================================================
std::unique_ptr<Sprite> Application::CreateSprite(const std::string& _path)
{
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>(deviceManager_->GetLogicalDevice(),
															  deviceManager_->GetPhysicalDevice(),
															  deviceManager_->GetQueueFamilyIndices());
	sprite->Create(_path, pipeline_->GetCommandPool(), descriptorSetLayout_, deviceManager_->GetGraphicsQueue());
	return sprite;
}
//======================================================================================================================
bool Application::InitVulkan()
{
	if(!CreateInstance())
	{
		return false;
	}
	if(!CreateSurface())
	{
		return false;
	}

	deviceManager_ = std::make_unique<DeviceManager>(instance_.get(), surface_.get());
	if(!deviceManager_->Create())
	{
		return false;
	}

	if(!CreateSwapChain())
	{
		return false;
	}
	if(!CreateDescriptorSetLayout())
	{
		return false;
	}
	if(!CreatePipeline())
	{
		return false;
	}
	if(!CreateFramebuffers())
	{
		return false;
	}
	if(!CreateDescriptorPool())
	{
		return false;
	}

	return true;
}
//======================================================================================================================
bool Application::CreateInstance()
{
	instance_		= std::make_unique<Instance>();
	bool isCreated	= instance_->Create();
	return isCreated;
}
//======================================================================================================================
bool Application::CreateSurface()
{
	surface_		= std::make_unique<Surface>(instance_.get(), window_->GetWindow());
	bool isCreated	= surface_->Create();
	return isCreated;
}
//======================================================================================================================
bool Application::CreateSwapChain()
{
	swapChain_	= std::make_unique<Swapchain>(deviceManager_->GetLogicalDevice(),
											  deviceManager_->GetPhysicalDevice(),
											  surface_.get(),
											  window_);
	bool result = swapChain_->Create() && swapChain_->CreateImageViews() && swapChain_->CreateDepthImageViews();
	return result;
}
//======================================================================================================================
bool Application::CreatePipeline()
{
	pipeline_ = std::make_unique<Pipeline>(deviceManager_->GetLogicalDevice(),
										   deviceManager_->GetPhysicalDevice(),
										   swapChain_.get(),
										   deviceManager_->GetQueueFamilyIndices(),
										   window_);
	if(!pipeline_->Create())
	{
		return false;
	}
	return true;
}

//======================================================================================================================
bool Application::CreateDescriptorSetLayout()
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

	if (vkCreateDescriptorSetLayout(deviceManager_->GetLogicalDevice(), &layoutInfo, nullptr, &descriptorSetLayout_) != VK_SUCCESS)
	{
		std::cout << "failed to create descriptor set layout!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool Application::CreateFramebuffers()
{
	return swapChain_->CreateFramebuffers(pipeline_->GetRenderPass()->GetRenderPass());
}
//======================================================================================================================
bool Application::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);
	poolSizes[1].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount	= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType			= VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount	= static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes		= poolSizes.data();
	poolInfo.maxSets		= static_cast<uint32_t>(MAX_FRAMES_IN_FLIGHT);

	if (vkCreateDescriptorPool(deviceManager_->GetLogicalDevice(), &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
	{
		std::cout << "failed to create descriptor pool!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
VkShaderModule  Application::CreateShaderModule(const std::vector<char>& _code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize	= _code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>(_code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(deviceManager_->GetLogicalDevice(), &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
//======================================================================================================================
void Application::MainLoop()
{
	while (!glfwWindowShouldClose(window_->GetWindow()))
	{
		glfwPollEvents();
		if(!DrawFrame())
		{
			std::cout << "Rendering failed, exiting main loop\n";
			break;
		}
	}

	vkDeviceWaitIdle(deviceManager_->GetLogicalDevice());
}
//======================================================================================================================
bool Application::DrawFrame()
{
	return pipeline_->RenderFrame(sprites_,
								  deviceManager_->GetGraphicsQueue(),
								  deviceManager_->GetPresentQueue());
}
//======================================================================================================================
void Application::Cleanup()
{
	// 1. Clear sprites first - they depend on descriptorSetLayout_ and logicalDevice_
	sprites_.clear();

	// 2. Destroy swapchain and pipeline
	swapChain_.reset();
	pipeline_.reset();

	// 3. Destroy descriptor resources
	vkDestroyDescriptorPool(deviceManager_->GetLogicalDevice(), descriptorPool_, nullptr);
	vkDestroyDescriptorSetLayout(deviceManager_->GetLogicalDevice(), descriptorSetLayout_, nullptr);

	// 4. Destroy pipeline layout
	vkDestroyPipelineLayout(deviceManager_->GetLogicalDevice(), pipelineLayout_, nullptr);

	// 5. Destroy device manager (destroys logical device)
	deviceManager_.reset();

	// 6. Destroy surface (device depends on surface, so device destroyed first)
	surface_.reset();

	// 7. Finally destroy instance (must be last)
	instance_.reset();
}
//======================================================================================================================
VkSurfaceFormatKHR Application::ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& _availableFormats)
{
	for (const auto& availableFormat : _availableFormats)
	{
		if (availableFormat.format == VK_FORMAT_R8G8B8A8_SRGB && availableFormat.colorSpace == VK_COLOR_SPACE_SRGB_NONLINEAR_KHR)
		{
			return availableFormat;
		}
	}

	return _availableFormats[0];
}
//======================================================================================================================
VkPresentModeKHR Application::ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& _availablePresentModes)
{
	for (const auto& availablePresentMode : _availablePresentModes)
	{
		if (availablePresentMode == VK_PRESENT_MODE_MAILBOX_KHR)
		{
			return availablePresentMode;
		}
	}

	return VK_PRESENT_MODE_FIFO_KHR;
}
//======================================================================================================================
VkExtent2D Application::ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& _capabilities)
{
	if (_capabilities.currentExtent.width != std::numeric_limits<uint32_t>::max())
	{
		return _capabilities.currentExtent;
	}
	else
	{
		int width, height;
		glfwGetFramebufferSize(window_->GetWindow(), &width, &height);

		VkExtent2D actualExtent =
		{
			static_cast<uint32_t>(width),
			static_cast<uint32_t>(height)
		};

		actualExtent.width	= std::clamp(actualExtent.width, _capabilities.minImageExtent.width, _capabilities.maxImageExtent.width);
		actualExtent.height	= std::clamp(actualExtent.height, _capabilities.minImageExtent.height, _capabilities.maxImageExtent.height);

		return actualExtent;
	}
}
}
