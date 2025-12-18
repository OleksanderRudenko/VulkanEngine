#pragma once

#include "sprite.h"
#include "tools.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>

namespace xengine
{

class Sprite;
class Swapchain;
class GraphicsPipeline;
class ResourceManager;
class ImGuiManager;

class ENGINE_API RenderPass
{
public:
	RenderPass(VkDevice logicalDevice,
			   VkPhysicalDevice physicalDevice,
			   Swapchain* swapchain,
			   ResourceManager* resourceManager,
			   ImGuiManager* imguiManager = nullptr);
	RenderPass(const RenderPass&)				= delete;
	RenderPass(RenderPass&&)					= delete;
	~RenderPass();

	RenderPass&	operator=(const RenderPass&)	= delete;
	RenderPass&	operator=(RenderPass&&)			= delete;

	bool				Create();
	bool				Render(VkCommandBuffer,
							   uint32_t imageIndex,
							   const std::vector<std::shared_ptr<Sprite>>&);
	void				Cleanup();

	void				SetImGuiManager(ImGuiManager* imguiManager) { imguiManager_ = imguiManager; }
	const VkRenderPass&	GetRenderPass()		const { return renderPass_; }

private:
	VkDevice										logicalDevice_;
	VkPhysicalDevice								physicalDevice_;
	Swapchain*										swapChain_;
	ResourceManager*								resourceManager_;
	ImGuiManager*									imguiManager_;

	VkRenderPass									renderPass_			= VK_NULL_HANDLE;
	std::unique_ptr<GraphicsPipeline>				graphicsPipeline_;
};

}