#include "stdafx.h"
#include "application.h"
#include "vertex.h"
#include "uniform.h"
#include "tools/timer.h"
#include <algorithm> 
#include <set>

namespace xengine
{

const std::vector<const char*> deviceExtensions =
{
	VK_KHR_SWAPCHAIN_EXTENSION_NAME
};

const int MAX_FRAMES_IN_FLIGHT = 2;

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
	std::unique_ptr<Sprite> sprite = std::make_unique<Sprite>(logicalDevice_, physicalDevice_, indices_);
	sprite->Create(_path, pipeline_->GetCommandPool(), descriptorSetLayout_, graphicsQueue_);
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
	PickPhysicalDevice();
	CreateLogicalDevice();

	if(!CreateSwapChain())
	{
		return false;
	}
	CreateDescriptorSetLayout();
	CreatePipeline();

	if(!CreateFramebuffers())
	{
		return false;
	}
	CreateDescriptorPool();

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
	surface_		= std::make_unique<Surface>(*instance_, window_->GetWindow());
	bool isCreated	= surface_->Create();
	return isCreated;
}
//======================================================================================================================
void Application::PickPhysicalDevice()
{
	uint32_t deviceCount = 0;
	vkEnumeratePhysicalDevices(instance_.get()->GetInstance(), &deviceCount, nullptr);
	if (deviceCount == 0)
	{
		throw std::runtime_error("failed to find GPUs with Vulkan support!");
	}

	std::vector<VkPhysicalDevice> devices(deviceCount);
	vkEnumeratePhysicalDevices(instance_.get()->GetInstance(), &deviceCount, devices.data());
	for (const auto& device : devices)
	{
		indices_ = surface_.get()->FindQueueFamilies(device);
		if (IsDeviceSuitable_(device))
		{
			physicalDevice_ = device;
			break;
		}
	}

	if (physicalDevice_ == VK_NULL_HANDLE)
	{
		throw std::runtime_error("failed to find a suitable GPU!");
	}
}
//======================================================================================================================
void Application::CreateLogicalDevice()
{
	std::vector<VkDeviceQueueCreateInfo> queueCreateInfos;
	std::set<uint32_t> uniqueQueueFamilies = {indices_.graphicsFamily.value(), indices_.presentFamily.value()};

	float queuePriority = 1.0f;
	for (uint32_t queueFamily : uniqueQueueFamilies)
	{
		VkDeviceQueueCreateInfo queueCreateInfo{};
		queueCreateInfo.sType				= VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO;
		queueCreateInfo.queueFamilyIndex	= queueFamily;
		queueCreateInfo.queueCount			= 1;
		queueCreateInfo.pQueuePriorities	= &queuePriority;
		queueCreateInfos.push_back(queueCreateInfo);
	}

	VkPhysicalDeviceFeatures deviceFeatures{};
	deviceFeatures.samplerAnisotropy = VK_TRUE;

	VkDeviceCreateInfo deviceCreateInfo{};
	deviceCreateInfo.sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO;

	deviceCreateInfo.queueCreateInfoCount = static_cast<uint32_t>(queueCreateInfos.size());
	deviceCreateInfo.pQueueCreateInfos = queueCreateInfos.data();

	deviceCreateInfo.pEnabledFeatures = &deviceFeatures;

	deviceCreateInfo.enabledExtensionCount = static_cast<uint32_t>(deviceExtensions.size());
	deviceCreateInfo.ppEnabledExtensionNames = deviceExtensions.data();

	if (instance_.get()->IsValidationLayersEnabled())
	{
		deviceCreateInfo.enabledLayerCount		= static_cast<uint32_t>(instance_.get()->GetValidationLayers().size());
		deviceCreateInfo.ppEnabledLayerNames	= instance_.get()->GetValidationLayers().data();
	}
	else
	{
		deviceCreateInfo.enabledLayerCount = 0;
	}

	if (vkCreateDevice(physicalDevice_, &deviceCreateInfo, nullptr, &logicalDevice_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create logical device!");
	}

	vkGetDeviceQueue(logicalDevice_, indices_.graphicsFamily.value(), 0, &graphicsQueue_);
	vkGetDeviceQueue(logicalDevice_, indices_.presentFamily.value(), 0, &presentQueue_);
}
//======================================================================================================================
bool Application::CreateSwapChain()
{
	swapChain_	= std::make_unique<Swapchain>(std::ref(logicalDevice_), std::ref(physicalDevice_), std::ref(*surface_), window_);
	bool result = swapChain_.get()->Create() && swapChain_.get()->CreateImageViews() && swapChain_.get()->CreateDepthImageViews();
	return result;
}
//======================================================================================================================
bool Application::CreatePipeline()
{
	pipeline_ = std::make_unique<Pipeline>(std::ref(logicalDevice_),
										   std::ref(physicalDevice_),
										   std::ref(*swapChain_),
										   std::ref(indices_),
										   window_);
	if(!pipeline_.get()->Create())
	{
		return false;
	}
	return true;
}

//======================================================================================================================
void Application::CreateDescriptorSetLayout()
{
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
		throw std::runtime_error("failed to create descriptor set layout!");
	}
}
//======================================================================================================================
bool Application::CreateFramebuffers()
{
	return swapChain_.get()->CreateFramebuffers(pipeline_->GetRenderPass()->GetRenderPass());
}
//======================================================================================================================
void Application::CreateDescriptorPool()
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

	if (vkCreateDescriptorPool(logicalDevice_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create descriptor pool!");
	}
}
//======================================================================================================================
VkShaderModule  Application::CreateShaderModule(const std::vector<char>& _code)
{
	VkShaderModuleCreateInfo createInfo{};
	createInfo.sType	= VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
	createInfo.codeSize	= _code.size();
	createInfo.pCode	= reinterpret_cast<const uint32_t*>(_code.data());

	VkShaderModule shaderModule;
	if (vkCreateShaderModule(logicalDevice_, &createInfo, nullptr, &shaderModule) != VK_SUCCESS)
	{
		throw std::runtime_error("failed to create shader module!");
	}

	return shaderModule;
}
//======================================================================================================================
void Application::MainLoop()
{
	while (!glfwWindowShouldClose(window_.get()->GetWindow()))
	{
		glfwPollEvents();
		DrawFrame();
	}

	vkDeviceWaitIdle(logicalDevice_);
}
//======================================================================================================================
void Application::DrawFrame()
{
	pipeline_->RenderFrame(sprites_,
						   graphicsQueue_,
						   presentQueue_);
}
//======================================================================================================================
void Application::Cleanup()
{
	swapChain_.reset();

	vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
	vkDestroyDescriptorSetLayout(logicalDevice_, descriptorSetLayout_, nullptr);

	vkDestroyPipelineLayout(logicalDevice_, pipelineLayout_, nullptr);

	surface_.reset();
	pipeline_.reset();

	sprites_.clear();

	vkDestroyDevice(logicalDevice_, nullptr);

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
		glfwGetFramebufferSize(window_.get()->GetWindow(), &width, &height);

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
//======================================================================================================================
bool Application::IsDeviceSuitable_(VkPhysicalDevice _device)
{
	bool extensionsSupported	= CheckDeviceExtensionSupport_(_device);

	bool swapChainAdequate		= false;
	if (extensionsSupported)
	{
		std::vector<VkSurfaceFormatKHR>	formats			= surface_.get()->GetFormats(_device);
		std::vector<VkPresentModeKHR>	presentModes	= surface_.get()->GetPresentModes(_device);
		swapChainAdequate = !formats.empty() && !presentModes.empty();
	}

	VkPhysicalDeviceFeatures supportedFeatures;
	vkGetPhysicalDeviceFeatures(_device, &supportedFeatures);

	return indices_.isComplete() && extensionsSupported && swapChainAdequate && supportedFeatures.samplerAnisotropy;
}
//======================================================================================================================
bool Application::CheckDeviceExtensionSupport_(VkPhysicalDevice _device)
{
	uint32_t extensionCount;
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, nullptr);

	std::vector<VkExtensionProperties> availableExtensions(extensionCount);
	vkEnumerateDeviceExtensionProperties(_device, nullptr, &extensionCount, availableExtensions.data());

	std::set<std::string> requiredExtensions(deviceExtensions.begin(), deviceExtensions.end());

	for (const auto& extension : availableExtensions)
	{
		std::cout << "Available extension: " << extension.extensionName << "\n";
		requiredExtensions.erase(extension.extensionName);
	}

	return requiredExtensions.empty();
}

}