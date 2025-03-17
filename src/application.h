#pragma once

#include "instance.h"
#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"
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

	bool			Init();
	void			Run();

private:
	bool			InitVulkan();
	bool			CreateInstance();
	bool			CreateSurface();
	void			PickPhysicalDevice();
	void			CreateLogicalDevice();
	bool			CreateSwapChain();
	void			CreateRenderPass();
	void			CreateDescriptorSetLayout();
	void			CreateGraphicsPipeline();
	bool			CreateFramebuffers();
	void			CreateCommandPool();
	bool			CreateTextureImage();
	bool			CreateTextureSampler();
	void			CreateVertexBuffer();
	void			CreateIndexBuffer();
	void			CreateUniformBuffers();
	void			CreateDescriptorPool();
	void			CreateDescriptorSets();
	bool			CreateCommandBuffer();
	void			CreateSyncObjects();
	void			RecreateSwapChain();
	
	VkShaderModule 	CreateShaderModule(const std::vector<char>& code);

	void			MainLoop();
	void			DrawFrame();
	void			Cleanup();
	void			UpdateUniformBuffer(uint32_t currentImage);

	void			RecordCommandBuffer(VkCommandBuffer commandBuffer, uint32_t imageIndex);

	// Tool functions
	static std::vector<char>	ReadFile_(const std::string& filename);

	// Swap chain functions
	VkSurfaceFormatKHR			ChooseSwapSurfaceFormat(const std::vector<VkSurfaceFormatKHR>& availableFormats);
	VkPresentModeKHR			ChooseSwapPresentMode(const std::vector<VkPresentModeKHR>& availablePresentModes);
	VkExtent2D					ChooseSwapExtent(const VkSurfaceCapabilitiesKHR& capabilities);

	bool			IsDeviceSuitable_(VkPhysicalDevice);
	bool			CheckDeviceExtensionSupport_(VkPhysicalDevice);

	std::unique_ptr<Window>								window_;
	std::unique_ptr<Instance>							instance_;
	VkPhysicalDevice									physicalDevice_			= VK_NULL_HANDLE;
	VkDevice											logicalDevice_			= VK_NULL_HANDLE;
	VkQueue												graphicsQueue_			= VK_NULL_HANDLE;

	std::unique_ptr<Surface>							surface_;

	VkQueue												presentQueue_			= VK_NULL_HANDLE;

	std::unique_ptr<Swapchain>							swapChain_;

	VkPipelineLayout									pipelineLayout_			= VK_NULL_HANDLE;
	VkDescriptorSetLayout								descriptorSetLayout_	= VK_NULL_HANDLE;
	VkRenderPass										renderPass_				= VK_NULL_HANDLE;
	VkPipeline											graphicsPipeline_		= VK_NULL_HANDLE;

	std::vector<std::unique_ptr<CommandBuffer>>			commandBuffers_;
	std::unique_ptr<CommandPool>						commandPool_;

	VkDescriptorPool									descriptorPool_			= VK_NULL_HANDLE;
	std::vector<VkDescriptorSet>						descriptorSets_;

	std::vector<VkSemaphore>							imageAvailableSemaphores_;
	std::vector<VkSemaphore>							renderFinishedSemaphores_;
	std::vector<VkFence>								inFlightFences_;
	uint32_t											currentFrame_			= 0;

	std::unique_ptr<Buffer>								vertexBuffer_;
	std::unique_ptr<Buffer>								indexBuffer_;
	std::vector<std::unique_ptr<Buffer>>				uniformBuffers_;
	std::vector<void*>									uniformBuffersMapped_;

	QueueFamilyIndices									indices_;

	VkDebugUtilsMessengerEXT							debugMessenger_			= VK_NULL_HANDLE;


	//test
	VkSampler textureSampler_;
	VkImageView textureImageView_;
	std::unique_ptr<Texture> texture_;
	// vector sprites
	// pipeline

};

}