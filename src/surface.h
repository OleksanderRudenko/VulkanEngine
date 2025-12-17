#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <functional>
#include <optional>

namespace xengine
{

class Instance;
class Window;

class ENGINE_API Surface
{
public:
	Surface(Instance* instance,
			GLFWwindow* window);
	Surface(const Surface&)					= delete;
	Surface(Surface&&)						= delete;
	~Surface();

	Surface&	operator=(const Surface&)	= delete;
	Surface&	operator=(Surface&&)		= delete;

	bool									Create();
	VkSurfaceKHR							GetSurface()	const	{ return surface_; }
	VkSurfaceCapabilitiesKHR				GetCapabilities(VkPhysicalDevice);
	std::vector<VkSurfaceFormatKHR>			GetFormats(VkPhysicalDevice);
	std::vector<VkPresentModeKHR>			GetPresentModes(VkPhysicalDevice);
	QueueFamilyIndices						FindQueueFamilies(VkPhysicalDevice);

	static VkSurfaceFormatKHR				ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	static VkPresentModeKHR					ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	static VkExtent2D						ChooseSwapExtent(const VkSurfaceCapabilitiesKHR&,
															 const Window&);

private:
	Instance*								instance_;
	GLFWwindow*								window_		= nullptr;
	VkSurfaceKHR							surface_	= VK_NULL_HANDLE;

	VkSurfaceCapabilitiesKHR				capabilities_;
	std::vector<VkSurfaceFormatKHR>			formats_;
	std::vector<VkPresentModeKHR>			presentModes_;
};

}