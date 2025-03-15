#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <GLFW/glfw3.h>
#include <functional>

namespace xengine
{

class Instance;

class ENGINE_API Surface
{
public:
	Surface(std::reference_wrapper<Instance>,
			GLFWwindow* window);
	Surface(const Surface&)					= delete;
	Surface(Surface&&)						= delete;
	~Surface();

	Surface&	operator=(const Surface&)	= delete;
	Surface&	operator=(Surface&&)		= delete;

	bool									Create();
	VkSurfaceKHR							GetSurface()																const	{ return surface_; }
	VkSurfaceCapabilitiesKHR				GetCapabilities(std::reference_wrapper<VkPhysicalDevice>);
	std::vector<VkSurfaceFormatKHR>			GetFormats(std::reference_wrapper<VkPhysicalDevice>);
	std::vector<VkPresentModeKHR>			GetPresentModes(std::reference_wrapper<VkPhysicalDevice>);

private:
	const std::reference_wrapper<Instance>	instance_;
	GLFWwindow*								window_		= nullptr;
	VkSurfaceKHR							surface_	= VK_NULL_HANDLE;

	VkSurfaceCapabilitiesKHR				capabilities_;
	std::vector<VkSurfaceFormatKHR>			formats_;
	std::vector<VkPresentModeKHR>			presentModes_;
};

}