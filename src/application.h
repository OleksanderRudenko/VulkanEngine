#pragma once

#include "buffer.h"
#include "tools.h"
#include "vulkan_engine_lib.h"
#include "window.h"
#include <iostream>
#include <stdexcept>
#include <cstdlib>

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

	bool			Init();
	void			Run();

private:
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
	void			CreateTextureImage();
	void			CreateVertexBuffer();
	void			CreateIndexBuffer();
	void			CreateUniformBuffers();
	void			CreateDescriptorPool();
	void			CreateDescriptorSets();
	void			CreateCommandBuffer();
	void			CreateSyncObjects();
	void			RecreateSwapChain();
	
	VkShaderModule 	CreateShaderModule(const std::vector<char>& code);
	//void			CreateBuffer(VkDeviceSize			size,
	//							 VkBufferUsageFlags		usage,
	//							 VkMemoryPropertyFlags	properties,
	//							 VkBuffer&				buffer,
	//							 VkDeviceMemory&		bufferMemory);
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
	SwapChainSupportDetails		QuerySwapChainSupport(VkPhysicalDevice);
	std::vector<const char*>	GetRequiredExtensions_();
	void						PopulateDebugMessengerCreateInfo_(VkDebugUtilsMessengerCreateInfoEXT&);
	static VKAPI_ATTR VkBool32 VKAPI_CALL DebugCallback_(VkDebugUtilsMessageSeverityFlagBitsEXT			messageSeverity,
														 VkDebugUtilsMessageTypeFlagsEXT				messageType,
														 const VkDebugUtilsMessengerCallbackDataEXT*	pCallbackData,
														 void*											pUserData);
	static std::vector<char>	ReadFile_(const std::string& filename);

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool			IsDeviceSuitable_(VkPhysicalDevice);
	bool			CheckDeviceExtensionSupport_(VkPhysicalDevice);

	std::unique_ptr<Window>								window_;
	VkInstance											instance_				= VK_NULL_HANDLE;
	VkPhysicalDevice									physicalDevice_			= VK_NULL_HANDLE;
	VkDevice											logicalDevice_					= VK_NULL_HANDLE;
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

	VkDescriptorPool									descriptorPool_			= VK_NULL_HANDLE;
	std::vector<VkDescriptorSet>						descriptorSets_;

	std::vector<VkSemaphore>							imageAvailableSemaphores_;
	std::vector<VkSemaphore>							renderFinishedSemaphores_;
	std::vector<VkFence>								inFlightFences_;
	uint32_t											currentFrame_			= 0;

	std::unique_ptr<Buffer>								vertexBuffer_;
	std::unique_ptr<Buffer>								indexBuffer_;
	// todo: buffer class
	//VkBuffer											vertexBuffer_			= VK_NULL_HANDLE;
	//VkDeviceMemory										vertexBufferMemory_		= VK_NULL_HANDLE;
	//VkBuffer											indexBuffer_			= VK_NULL_HANDLE;
	//VkDeviceMemory										indexBufferMemory_		= VK_NULL_HANDLE;

	std::vector<std::unique_ptr<Buffer>>				uniformBuffers_;
	//std::vector<VkDeviceMemory>							uniformBuffersMemory_;
	std::vector<void*>									uniformBuffersMapped_;

	VkDebugUtilsMessengerEXT							debugMessenger_			= VK_NULL_HANDLE;;
};