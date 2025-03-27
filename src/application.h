#pragma once

#include "instance.h"
#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "pipeline.h"
#include "sprite.h"
#include "surface.h"
#include "swapchain.h"
#include "texture.h"
#include "vulkan_engine_lib.h"
#include "window.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

namespace xengine
{

class ENGINE_API Application
{
public:
	Application(uint32_t width,
				uint32_t height);
	Application(const Application&)					= delete;
	Application(Application&&)						= delete;
	virtual ~Application()							= default;

	Application&	operator=(const Application&)	= delete;
	Application&	operator=(Application&&)		= delete;

	bool					Init();
	void					Run();

	std::unique_ptr<Sprite>	CreateSprite(const std::string& path);
	void					AddSprite(std::shared_ptr<Sprite> _sprite) { sprites_.push_back(_sprite); }

private:
	bool			InitVulkan();
	bool			CreateInstance();
	bool			CreateSurface();
	void			PickPhysicalDevice();
	void			CreateLogicalDevice();
	bool			CreateSwapChain();
	bool			CreatePipeline();

	void			CreateDescriptorSetLayout();
	bool			CreateFramebuffers();
	void			CreateCommandPool();

	void			CreateDescriptorPool();
	
	VkShaderModule 	CreateShaderModule(const std::vector<char>& code);

	void			MainLoop();
	void			DrawFrame();
	void			Cleanup();

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool			IsDeviceSuitable_(VkPhysicalDevice);
	bool			CheckDeviceExtensionSupport_(VkPhysicalDevice);

	std::shared_ptr<Window>								window_;
	std::unique_ptr<Instance>							instance_;
	VkPhysicalDevice									physicalDevice_			= VK_NULL_HANDLE;
	VkDevice											logicalDevice_			= VK_NULL_HANDLE;
	VkQueue												graphicsQueue_			= VK_NULL_HANDLE;

	std::unique_ptr<Surface>							surface_;

	VkQueue												presentQueue_			= VK_NULL_HANDLE;

	std::unique_ptr<Swapchain>							swapChain_;

	VkPipelineLayout									pipelineLayout_			= VK_NULL_HANDLE;
	VkDescriptorSetLayout								descriptorSetLayout_	= VK_NULL_HANDLE;
	VkPipeline											graphicsPipeline_		= VK_NULL_HANDLE;

	VkDescriptorPool									descriptorPool_			= VK_NULL_HANDLE;

	QueueFamilyIndices									indices_;

	std::vector<std::shared_ptr<Sprite>>				sprites_;
	std::unique_ptr<Pipeline>							pipeline_;
};

}