#pragma once

#include "instance.h"
#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"
#include "device_manager.h"
#include "input_handler.h"
#include "pipeline.h"
#include "resource_manager.h"
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
	virtual ~Application();

	Application&	operator=(const Application&)	= delete;
	Application&	operator=(Application&&)		= delete;

	bool					Init();

	std::shared_ptr<Sprite>	CreateSprite(const std::string& path);

	InputHandler*			GetInputHandler()	const	{ return inputHandler_.get(); }

	// Main loop functions
	void					DeviceWaitIdle();
	bool					ShouldClose()		const;
	void					GLFWPollEvents()	const;
	bool					DrawFrame();

private:
	bool					InitVulkan();
	bool					CreateInstance();
	bool					CreateSurface();
	bool					CreateSwapChain();
	bool					CreatePipeline();
	void					Cleanup();

	bool					CreateFramebuffers();

	VkShaderModule			CreateShaderModule(const std::vector<char>& code);

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	std::shared_ptr<Window>								window_;
	std::unique_ptr<InputHandler>						inputHandler_;
	std::unique_ptr<Instance>							instance_;
	std::unique_ptr<Surface>							surface_;
	std::unique_ptr<DeviceManager>						deviceManager_;
	std::unique_ptr<ResourceManager>					resourceManager_;

	std::unique_ptr<Swapchain>							swapChain_;

	VkPipeline											graphicsPipeline_		= VK_NULL_HANDLE;

	std::vector<std::shared_ptr<Sprite>>				sprites_;
	std::unique_ptr<Pipeline>							pipeline_;
};

}