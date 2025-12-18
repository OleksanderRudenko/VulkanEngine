#pragma once

#include "vulkan_engine_lib.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <memory>

namespace xengine
{

class DeviceManager;
class Window;

class ENGINE_API ImGuiManager
{
public:
	ImGuiManager(VkInstance instance,
				 VkDevice logicalDevice,
				 VkPhysicalDevice physicalDevice,
				 uint32_t queueFamily,
				 VkQueue graphicsQueue,
				 VkRenderPass renderPass,
				 uint32_t imageCount);
	ImGuiManager(const ImGuiManager&)			= delete;
	ImGuiManager(ImGuiManager&&)				= delete;
	~ImGuiManager();

	ImGuiManager&	operator=(const ImGuiManager&)	= delete;
	ImGuiManager&	operator=(ImGuiManager&&)		= delete;

	bool			Init(GLFWwindow* window);
	void			NewFrame();
	void			Render(VkCommandBuffer commandBuffer);
	void			Shutdown();

private:
	VkInstance			instance_;
	VkDevice			logicalDevice_;
	VkPhysicalDevice	physicalDevice_;
	uint32_t			queueFamily_;
	VkQueue				graphicsQueue_;
	VkRenderPass		renderPass_;
	uint32_t			imageCount_;

	VkDescriptorPool	descriptorPool_		= VK_NULL_HANDLE;
	bool				initialized_		= false;
};

}
