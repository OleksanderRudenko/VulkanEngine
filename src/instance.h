#pragma once

#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <vector>
#include <string>
#include <functional>

namespace xengine
{

class ENGINE_API Instance
{
public:
	Instance()								= default;
	Instance(const Instance&)				= delete;
	Instance(Instance&&)					= delete;
	~Instance();

	Instance& operator=(const Instance&)	= delete;
	Instance& operator=(Instance&&)			= delete;
	operator const VkInstance& () const		{ return instance_; }

	bool									Create();
	const VkInstance&						GetInstance()				const { return instance_; }
	bool									CheckValidationLayerSupport();
	const std::vector<const char*>&			GetValidationLayers()		const { return validationLayers; }
	bool 									IsValidationLayersEnabled()	const { return enableValidationLayers; }

private:
	bool									CreateInstance();
	bool									SetupDebugMessenger();
	void									PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT&);
	static VKAPI_ATTR VkBool32 VKAPI_CALL	DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT,
														  VkDebugUtilsMessageTypeFlagsEXT,
														  const VkDebugUtilsMessengerCallbackDataEXT*,
														  void* pUserData);

	std::vector<const char*>				GetRequiredExtensions_();
	VkResult								CreateDebugUtilsMessengerEXT_(const VkDebugUtilsMessengerCreateInfoEXT*	_pCreateInfo,
																		  const VkAllocationCallbacks*				_pAllocator,
																		  VkDebugUtilsMessengerEXT*					_pDebugMessenger);

	VkInstance					instance_		= VK_NULL_HANDLE;
	VkDebugUtilsMessengerEXT	debugMessenger_	= VK_NULL_HANDLE;

	#ifdef NDEBUG
	const bool enableValidationLayers = false;
	#else
	const bool enableValidationLayers = true;
	#endif

	const std::vector<const char*> validationLayers =
	{
		"VK_LAYER_KHRONOS_validation"
	};
};

}