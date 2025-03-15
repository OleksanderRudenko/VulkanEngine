#include "Instance.h"
#include <stdexcept>
#include <iostream>
#include <GLFW/glfw3.h>

namespace xengine
{

//======================================================================================================================
Instance::~Instance() {
	if (debugMessenger_ != VK_NULL_HANDLE)
	{
		auto func = (PFN_vkDestroyDebugUtilsMessengerEXT)vkGetInstanceProcAddr(instance_, "vkDestroyDebugUtilsMessengerEXT");
		if (func != nullptr)
		{
			func(instance_, debugMessenger_, nullptr);
		}
	}
	vkDestroyInstance(instance_, nullptr);
}
//======================================================================================================================
bool Instance::Create()
{
	return CreateInstance() && SetupDebugMessenger();
}
//======================================================================================================================
bool Instance::CreateInstance()
{
	if (enableValidationLayers && !CheckValidationLayerSupport())
	{
		std::cout << "validation layers requested, but not available!\n";
		return false;
	}

	VkApplicationInfo appInfo{};
	appInfo.sType				= VK_STRUCTURE_TYPE_APPLICATION_INFO;
	appInfo.pApplicationName	= "Hello Triangle";
	appInfo.applicationVersion	= VK_MAKE_VERSION(1, 0, 0);
	appInfo.pEngineName			= "No Engine";
	appInfo.engineVersion		= VK_MAKE_VERSION(1, 0, 0);
	appInfo.apiVersion			= VK_API_VERSION_1_0;

	VkInstanceCreateInfo createInfo{};
	createInfo.sType			= VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO;
	createInfo.pApplicationInfo	= &appInfo;

	auto extensions						= GetRequiredExtensions_();
	createInfo.enabledExtensionCount	= static_cast<uint32_t>(extensions.size());
	createInfo.ppEnabledExtensionNames	= extensions.data();

	VkDebugUtilsMessengerCreateInfoEXT debugCreateInfo{};
	if (enableValidationLayers)
	{
		createInfo.enabledLayerCount = static_cast<uint32_t>(validationLayers.size());
		createInfo.ppEnabledLayerNames = validationLayers.data();

		PopulateDebugMessengerCreateInfo(debugCreateInfo);
		createInfo.pNext = (VkDebugUtilsMessengerCreateInfoEXT*) &debugCreateInfo;
	}
	else
	{
		createInfo.enabledLayerCount	= 0;
		createInfo.pNext				= nullptr;
	}

	if(vkCreateInstance(&createInfo, nullptr, &instance_) != VK_SUCCESS)
	{
		std::cout << "failed to create instance!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool Instance::SetupDebugMessenger()
{
	if (!enableValidationLayers) return true;

	VkDebugUtilsMessengerCreateInfoEXT createInfo;
	PopulateDebugMessengerCreateInfo(createInfo);

	if (CreateDebugUtilsMessengerEXT_(&createInfo, nullptr, &debugMessenger_) != VK_SUCCESS)
	{
		std::cout << "failed to set up debug messenger!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
void Instance::PopulateDebugMessengerCreateInfo(VkDebugUtilsMessengerCreateInfoEXT& _createInfo)
{
	_createInfo					= {};
	_createInfo.sType			= VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT;
	_createInfo.messageSeverity	= VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT;
	_createInfo.messageType		= VK_DEBUG_UTILS_MESSAGE_TYPE_GENERAL_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT;
	_createInfo.pfnUserCallback	= DebugCallback;
}
//======================================================================================================================
VKAPI_ATTR VkBool32 VKAPI_CALL Instance::DebugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT		_messageSeverity,
													   VkDebugUtilsMessageTypeFlagsEXT				_messageType,
													   const VkDebugUtilsMessengerCallbackDataEXT*	_pCallbackData,
													   void* pUserData) {
	std::cerr << "validation layer: " << _pCallbackData->pMessage << std::endl;

	return VK_FALSE;
}
//======================================================================================================================
bool Instance::CheckValidationLayerSupport()
{
	uint32_t layerCount;
	vkEnumerateInstanceLayerProperties(&layerCount, nullptr);

	std::vector<VkLayerProperties> availableLayers(layerCount);
	vkEnumerateInstanceLayerProperties(&layerCount, availableLayers.data());

	for (const char* layerName : validationLayers)
	{
		bool layerFound = false;
		for (const auto& layerProperties : availableLayers)
		{
			if (strcmp(layerName, layerProperties.layerName) == 0)
			{
				layerFound = true;
				break;
			}
		}

		if (!layerFound)
		{
			return false;
		}
	}

	return true;
}
//======================================================================================================================
std::vector<const char*> Instance::GetRequiredExtensions_()
{
	uint32_t glfwExtensionCount = 0;
	const char** glfwExtensions;
	glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

	std::vector<const char*> extensions(glfwExtensions, glfwExtensions + glfwExtensionCount);

	if (enableValidationLayers)
	{
		extensions.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
	}

	return extensions;
}
//======================================================================================================================
VkResult Instance::CreateDebugUtilsMessengerEXT_(const VkDebugUtilsMessengerCreateInfoEXT*	_pCreateInfo,
												 const VkAllocationCallbacks*				_pAllocator,
												 VkDebugUtilsMessengerEXT*					_pDebugMessenger)
{
	auto func = (PFN_vkCreateDebugUtilsMessengerEXT) vkGetInstanceProcAddr(instance_, "vkCreateDebugUtilsMessengerEXT");
	if (func != nullptr)
	{
		return func(instance_, _pCreateInfo, _pAllocator, _pDebugMessenger);
	}
	else
	{
		return VK_ERROR_EXTENSION_NOT_PRESENT;
	}
}

}