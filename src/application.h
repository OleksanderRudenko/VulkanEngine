#pragma once

#include "tools.h"
#include "vulkan_engine_lib.h"
#include <GLFW/glfw3.h>
#include <iostream>
#include <stdexcept>
#include <cstdlib>
#include <functional>

//**********************************************************************************************************************
//	eApplication
//----------------------------------------------------------------------------------------------------------------------
class ENGINE_API eApplication
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
	void			CreateDescriptorSetLayout();
	void			CreateGraphicsPipeline();
	void			CreateFramebuffers();
	void			CreateCommandPool();
	void			CreateVertexBuffer();
	void			CreateIndexBuffer();
	void			CreateUniformBuffers();
	void			CreateDescriptorPool();
	void			CreateDescriptorSets();
	void			CreateCommandBuffer();
	void			CreateSyncObjects();
	void			RecreateSwapChain();
	
	VkShaderModule 	CreateShaderModule(const std::vector<char>& code);
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
	void			UpdateUniformBuffer(uint32_t currentImage);

	void			RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);
	bool			CheckValidationLayerSupport();
	void			SetupDebugMessenger();

	// Tool functions
	QueueFamilyIndices			FindQueueFamilies(VkPhysicalDevice);
	uint32_t					FindMemoryType(uint32_t typeFilter, VkMemoryPropertyFlags);
	SwapChainSupportDetails		QuerySwapChainSupport(VkPhysicalDevice);
	std::vector<const char*>	GetRequiredExtensions_();
	void						PopulateDebugMessengerCreateInfo_(VkDebugUtilsMessengerCreateInfoEXT&);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT			messageSeverity,
														 VkDebugUtilsMessageTypeFlagsEXT				messageType,
														 const VkDebugUtilsMessengerCallbackDataEXT*	pCallbackData,
														 void*											pUserData);
	static void					FramebufferResizeCallback(GLFWwindow*	window,
														  int			width,
														  int			height);
	static std::vector<char>	ReadFile_(const std::string& filename);

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool			IsDeviceSuitable_(VkPhysicalDevice);
	bool			CheckDeviceExtensionSupport_(VkPhysicalDevice);

	std:: unique_ptr<GLFWwindow,
					 std::function<void(GLFWwindow*)>>	window_;
	VkInstance											instance_;
	VkPhysicalDevice									physicalDevice_			= VK_NULL_HANDLE;
	VkDevice											device_					= VK_NULL_HANDLE;
	VkQueue												graphicsQueue_			= VK_NULL_HANDLE;
	VkSurfaceKHR										surface_				= VK_NULL_HANDLE;
	VkQueue												presentQueue_			= VK_NULL_HANDLE;

	VkSwapchainKHR										swapChain_				= VK_NULL_HANDLE;
	std::vector<VkImage>								swapChainImages_;
	VkFormat											swapChainImageFormat_	= VK_FORMAT_UNDEFINED;
	VkExtent2D											swapChainExtent_;
	std::vector<VkImageView>							swapChainImageViews_;
	std::vector<VkFramebuffer>							swapChainFramebuffers_;

	VkPipelineLayout									pipelineLayout_			= VK_NULL_HANDLE;
	VkDescriptorSetLayout								descriptorSetLayout_	= VK_NULL_HANDLE;
	VkRenderPass										renderPass_				= VK_NULL_HANDLE;
	VkPipeline											graphicsPipeline_		= VK_NULL_HANDLE;

	VkCommandPool										commandPool_			= VK_NULL_HANDLE;
	std::vector<VkCommandBuffer>						commandBuffers_;

	VkDescriptorPool									descriptorPool_			= VK_NULL_HANDLE;;
	std::vector<VkDescriptorSet>						descriptorSets_;

	std::vector<VkSemaphore>							imageAvailableSemaphores_;
	std::vector<VkSemaphore>							renderFinishedSemaphores_;
	std::vector<VkFence>								inFlightFences_;
	uint32_t											currentFrame_			= 0;
	bool												framebufferResized_		= false;

	// todo: buffer class
	VkBuffer											vertexBuffer_			= VK_NULL_HANDLE;;
	VkDeviceMemory										vertexBufferMemory_		= VK_NULL_HANDLE;;
	VkBuffer											indexBuffer_			= VK_NULL_HANDLE;;
	VkDeviceMemory										indexBufferMemory_		= VK_NULL_HANDLE;;

	std::vector<VkBuffer>								uniformBuffers_;
	std::vector<VkDeviceMemory>							uniformBuffersMemory_;
	std::vector<void*>									uniformBuffersMapped_;

	VkDebugUtilsMessengerEXT							debugMessenger_			= VK_NULL_HANDLE;;
};