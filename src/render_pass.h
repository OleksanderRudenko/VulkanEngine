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

class ENGINE_API RenderPass
{
public:
	RenderPass(std::reference_wrapper<VkDevice> logicalDevice,
			   std::reference_wrapper<Swapchain>);
	RenderPass(const RenderPass&)				= delete;
	RenderPass(RenderPass&&)					= delete;
	~RenderPass();

	RenderPass&	operator=(const RenderPass&)	= delete;
	RenderPass&	operator=(RenderPass&&)			= delete;

	bool				Create();
	void				Render(VkCommandBuffer,
							   uint32_t imageIndex,
							   const std::vector<std::shared_ptr<Sprite>>&);
	void				Cleanup();

	const VkRenderPass&	GetRenderPass()		const { return renderPass_; }

private:
	const std::reference_wrapper<VkDevice>	logicalDevice_;
	const std::reference_wrapper<Swapchain>	swapChain_;

	VkRenderPass							renderPass_			= VK_NULL_HANDLE;
	std::unique_ptr<GraphicsPipeline>		graphicsPipeline_;
};

}