#pragma once

#include "tools.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>
#include <string>

namespace xengine
{

class CommandPool;
class CommandBuffer;
class Buffer;

class Texture
{
public:
	Texture(std::reference_wrapper<VkDevice>	logicalDevice,
			std::reference_wrapper<VkPhysicalDevice>,
			const QueueFamilyIndices&);

	Texture(const Texture&)					= default;
	Texture(Texture&&)						= default;
	virtual ~Texture();

	Texture&	operator=(const Texture&)	= default;
	Texture&	operator=(Texture&&)		= default;

	const VkImage&		GetImage()			{ return image_; }
	bool				Create(const std::string& path);
	void				TransitionImageLayout(VkFormat,
											  VkImageLayout	oldLayout,
											  VkImageLayout	newLayout,
											  CommandPool*,
											  VkQueue		graphicsQueue);
	void				CopyBufferToImage(CommandPool*,
										  VkQueue	graphicsQueue);
	static VkImageView	CreateTextureImageView(std::reference_wrapper<VkDevice>	logicalDevice,
											   const VkImage&,
											   VkFormat);
	static VkSampler	CreateTextureSampler(std::reference_wrapper<VkDevice>	logicalDevice,
											 std::reference_wrapper<VkPhysicalDevice>);

protected:
	const std::reference_wrapper<VkDevice>			logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>	physicalDevice_;
	const QueueFamilyIndices						indices_;

	VkImage											image_			= VK_NULL_HANDLE;
	VkDeviceMemory									imageMemory_	= VK_NULL_HANDLE;
	VkImageView										imageView_		= VK_NULL_HANDLE;
	VkSampler										sampler_		= VK_NULL_HANDLE;
	std::unique_ptr<CommandBuffer>					commandBuffer_;
	std::unique_ptr<Buffer>							stagingBuffer_;
	int												width_		= 0;
	int												height_		= 0;
};

}
