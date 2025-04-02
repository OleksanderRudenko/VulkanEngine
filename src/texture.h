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

	const VkImage&		GetImage()		const	{ return image_; }
	const VkImageView&	GetImageView()	const	{ return imageView_; }
	const VkSampler&	GetSampler()	const	{ return sampler_; }
	int					GetWidth()		const	{ return width_; }
	int					GetHeight()		const	{ return height_; }

	bool				Create(const std::string& path,
							   VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
							   VkImageUsageFlags flags = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT);
	bool				TransitionImageLayout(VkFormat,
											  VkImageLayout	oldLayout,
											  VkImageLayout	newLayout,
											  std::shared_ptr<CommandPool>,
											  VkQueue		graphicsQueue);
	void				CopyBufferToImage(std::shared_ptr<CommandPool>,
										  VkQueue	graphicsQueue);
	bool				CreateTextureImageView(VkFormat format = VK_FORMAT_R8G8B8A8_SRGB,
											   VkImageAspectFlags flags = VK_IMAGE_ASPECT_COLOR_BIT);
	bool				CreateTextureSampler();

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
