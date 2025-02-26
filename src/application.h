#pragma once

#include "tools.h"
#include <GLFW/glfw3.h>
#include <vulkan/vulkan.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>

//**********************************************************************************************************************
//	eApplication
//----------------------------------------------------------------------------------------------------------------------
class eApplication
{
public:
	eApplication()									= default;
	eApplication(const eApplication&)				= delete;
	eApplication(eApplication&&)					= default;
	virtual ~eApplication()							= default;

	eApplication&	operator=(const eApplication&)	= delete;
	eApplication&	operator=(eApplication&&)		= default;

	void			Run(uint32_t widht,
						uint32_t height);

private:
	void			InitWindow(uint32_t width,
							   uint32_t height);
	void			InitVulkan();
	void			CreateInstance();
	void			CreateSurface();
	void			PickPhysicalDevice();
	void			CreateLogicalDevice();
	void			CreateSwapChain();
	void			CreateImageViews();
	void			CreateRenderPass();
	void			CreateGraphicsPipeline();
	void			CreateFramebuffers();
	void			CreateCommandPool();
	void			CreateVertexBuffer();
	void			CreateIndexBuffer();
	void			CreateCommandBuffer();
	void			CreateSyncObjects();
	void			RecreateSwapChain();
	
	VkShaderModule 	CreateShaderModule(const vector<char>& code);
	void			CreateBuffer(VkDeviceSize			size,
								 VkBufferUsageFlags		usage,
								 VkMemoryPropertyFlags	properties,
								 VkBuffer&				buffer,
								 VkDeviceMemory&		bufferMemory);
	void			CopyBuffer(VkBuffer srcBuffer,
							   VkBuffer dstBuffer,
							   VkDeviceSize size);

	void			MainLoop();
	void			DrawFrame();
	void			Cleanup();
	void			CleanupSwapChain();

	void			RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	bool			CheckValidationLayerSupport();
	void			SetupDebugMessenger();

	// Tool functions
	QueueFamilyIndices			FindQueueFamilies(VkPhysicalDevice);
	uint32_t					FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);
	SwapChainSupportDetails		QuerySwapChainSupport(VkPhysicalDevice);
	vector<const char*>			GetRequiredExtensions_();
	void						PopulateDebugMessengerCreateInfo_(VkDebugUtilsMessengerCreateInfoEXT&);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT			messageSeverity,
														 VkDebugUtilsMessageTypeFlagsEXT				messageType,
														 const VkDebugUtilsMessengerCallbackDataEXT*	pCallbackData,
														 void*											pUserData);
	static void					FramebufferResizeCallback(GLFWwindow*	window,
														  int			width,
														  int			height);
	static vector<char>			ReadFile_(const std::string& filename);

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool			IsDeviceSuitable_(VkPhysicalDevice);
	bool			CheckDeviceExtensionSupport_(VkPhysicalDevice);

	unique_ptr<GLFWwindow, function<void(GLFWwindow*)>>	window_;
	VkInstance											instance_;
	VkPhysicalDevice									physicalDevice_		= VK_NULL_HANDLE;
	VkDevice											device_;
	VkQueue												graphicsQueue_;
	VkSurfaceKHR										surface_;
	VkQueue												presentQueue_;

	VkSwapchainKHR										swapChain_;
	vector<VkImage>										swapChainImages_;
	VkFormat											swapChainImageFormat_;
	VkExtent2D											swapChainExtent_;
	vector<VkImageView>									swapChainImageViews_;
	vector<VkFramebuffer>								swapChainFramebuffers_;

	VkPipelineLayout									pipelineLayout_;
	VkRenderPass										renderPass_;
	VkPipeline											graphicsPipeline_;

	VkCommandPool										commandPool_;
	vector<VkCommandBuffer>								commandBuffers_;

	vector<VkSemaphore>									imageAvailableSemaphores_;
	vector<VkSemaphore>									renderFinishedSemaphores_;
	vector<VkFence>										inFlightFences_;
	uint32_t											currentFrame_		= 0;
	bool												framebufferResized_	= false;

	VkBuffer											vertexBuffer_;
	VkDeviceMemory										vertexBufferMemory_;
	VkBuffer											indexBuffer_;
	VkDeviceMemory										indexBufferMemory_;

	VkDebugUtilsMessengerEXT							debugMessenger_;
};