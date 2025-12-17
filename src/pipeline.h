#pragma once

#include "render_pass.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace xengine
{

class Sprite;
class CommandBuffer;
class CommandPool;
class Swapchain;
class Window;
class ResourceManager;
struct QueueFamilyIndices;

class ENGINE_API Pipeline
{
public:
	Pipeline(VkDevice logicalDevice,
			 VkPhysicalDevice physicalDevice,
			 Swapchain* swapchain,
			 const QueueFamilyIndices& indices,
			 std::shared_ptr<Window>,
			 ResourceManager* resourceManager);
	Pipeline(const Pipeline&)				= delete;
	Pipeline(Pipeline&&)					= delete;
	~Pipeline();

	Pipeline&	operator=(const Pipeline&)	= delete;
	Pipeline&	operator=(Pipeline&&)		= delete;

	bool		Create();
	bool		RenderFrame(const std::vector<std::shared_ptr<Sprite>>&,
							VkQueue graphicsQueue,
							VkQueue	presentQueue);

	std::shared_ptr<RenderPass>		GetRenderPass()		const { return renderPass_; }
	std::shared_ptr<CommandPool>	GetCommandPool()	const { return commandPool_; }

private:
	bool		CreateSyncObjects();
	bool		CreateCommandBuffers();
	bool		RecordCommandBuffer(VkCommandBuffer,
									uint32_t imageIndex,
									const std::vector<std::shared_ptr<Sprite>>& _sprites);

	VkDevice										logicalDevice_;
	VkPhysicalDevice								physicalDevice_;
	Swapchain*										swapChain_;
	const QueueFamilyIndices&						indices_;
	const std::shared_ptr<Window>					window_;
	ResourceManager*								resourceManager_;

	std::vector<VkSemaphore>							imageAvailableSemaphores_;
	std::vector<VkSemaphore>							renderFinishedSemaphores_;
	std::vector<VkFence>								inFlightFences_;
	uint32_t											currentFrame_ = 0;

	std::shared_ptr<RenderPass>							renderPass_;
	std::vector<std::unique_ptr<CommandBuffer>>			commandBuffers_;
	std::shared_ptr<CommandPool>						commandPool_;
};

}