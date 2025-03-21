#include "stdafx.h"
#include "texture.h"
#include "buffer.h"
#include "command_buffer.h"
#include "command_pool.h"
#include <stb_image.h>
#include <iostream>

namespace xengine
{

//======================================================================================================================
Texture::Texture(std::reference_wrapper<VkDevice>			_logicalDevice,
				 std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
				 const QueueFamilyIndices&					_indices)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, indices_(_indices)
{}
//======================================================================================================================
Texture::~Texture()
{
	vkDestroyImage(logicalDevice_, image_, nullptr);
	vkDestroyImageView(logicalDevice_, imageView_, nullptr);
	vkDestroySampler(logicalDevice_, sampler_, nullptr);
	vkFreeMemory(logicalDevice_, imageMemory_, nullptr);

}
//======================================================================================================================
bool Texture::Create(const std::string& _path)
{
	int texChannels;
	stbi_uc* pixels = stbi_load(_path.c_str(), &width_, &height_, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = width_ * height_ * 4;

	if (!pixels)
	{
		std::cout<< "failed to load texture image!\n";
		return false;
	}

	stagingBuffer_ = std::make_unique<Buffer>(imageSize, std::ref(logicalDevice_));
	stagingBuffer_.get()->CreateBuffer(physicalDevice_,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	vkMapMemory(logicalDevice_, stagingBuffer_.get()->GetBufferMemory(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(logicalDevice_, stagingBuffer_.get()->GetBufferMemory());
	stbi_image_free(pixels);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageInfo.extent.width	= width_;
	imageInfo.extent.height	= height_;
	imageInfo.extent.depth	= 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling		= VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(logicalDevice_, &imageInfo, nullptr, &image_) != VK_SUCCESS)
	{
		std::cout<< "failed to create image!\n";
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(logicalDevice_, image_, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	= memRequirements.size;
	allocInfo.memoryTypeIndex	= Buffer::FindMemoryType(physicalDevice_, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(logicalDevice_, &allocInfo, nullptr, &imageMemory_) != VK_SUCCESS)
	{
		std::cout<< "failed to allocate image memory!\n";
		return false;
	}

	vkBindImageMemory(logicalDevice_, image_, imageMemory_, 0);

	return true;
}
//======================================================================================================================
bool Texture::TransitionImageLayout(VkFormat						_format,
									VkImageLayout					_oldLayout,
									VkImageLayout					_newLayout,
									std::shared_ptr<CommandPool>	_commandPool,
									VkQueue							_graphicsQueue)
{
	CommandBuffer commandBuffer(logicalDevice_, physicalDevice_, indices_);
	commandBuffer.Create(_commandPool);
	commandBuffer.Begin();

	VkImageMemoryBarrier barrier{};
	barrier.sType							= VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
	barrier.oldLayout						= _oldLayout;
	barrier.newLayout						= _newLayout;
	barrier.srcQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.dstQueueFamilyIndex				= VK_QUEUE_FAMILY_IGNORED;
	barrier.image							= image_;
	barrier.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	barrier.subresourceRange.baseMipLevel	= 0;
	barrier.subresourceRange.levelCount		= 1;
	barrier.subresourceRange.baseArrayLayer	= 0;
	barrier.subresourceRange.layerCount		= 1;
	barrier.srcAccessMask					= 0; // TODO
	barrier.dstAccessMask					= 0; // TODO

	VkPipelineStageFlags sourceStage;
	VkPipelineStageFlags destinationStage;

	if (_oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && _newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL)
	{
		barrier.srcAccessMask	= 0;
		barrier.dstAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
		sourceStage				= VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
		destinationStage		= VK_PIPELINE_STAGE_TRANSFER_BIT;
	}
	else if (_oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && _newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL)
	{
		barrier.srcAccessMask	= VK_ACCESS_TRANSFER_WRITE_BIT;
		barrier.dstAccessMask	= VK_ACCESS_SHADER_READ_BIT;
		sourceStage				= VK_PIPELINE_STAGE_TRANSFER_BIT;
		destinationStage		= VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
	}
	else
	{
		std::cout<< "unsupported layout transition!\n";
		return false;
	}

	vkCmdPipelineBarrier(commandBuffer.GetBuffer(),
						 sourceStage,
						 destinationStage,
						 0,
						 0,
						 nullptr,
						 0,
						 nullptr,
						 1,
						 &barrier);
	commandBuffer.SubmitAndWaitIdle(_graphicsQueue);
	return true;
}
//======================================================================================================================
void Texture::CopyBufferToImage(std::shared_ptr<CommandPool>	_commandPool,
								VkQueue							_graphicsQueue)
{
	commandBuffer_ = std::make_unique<CommandBuffer>(logicalDevice_, physicalDevice_, indices_);
	commandBuffer_.get()->Create(_commandPool);
	commandBuffer_.get()->Begin();

	VkBufferImageCopy region{};
	region.bufferOffset			= 0;
	region.bufferRowLength		= 0;
	region.bufferImageHeight	= 0;

	region.imageSubresource.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	region.imageSubresource.mipLevel		= 0;
	region.imageSubresource.baseArrayLayer	= 0;
	region.imageSubresource.layerCount		= 1;
	region.imageOffset						= {0, 0, 0};
	region.imageExtent						= {static_cast<uint32_t>(width_), static_cast<uint32_t>(height_), 1};
	vkCmdCopyBufferToImage(commandBuffer_.get()->GetBuffer(), stagingBuffer_.get()->GetBuffer(), image_, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

	commandBuffer_.get()->SubmitAndWaitIdle(_graphicsQueue);
}
//======================================================================================================================
bool Texture::CreateTextureImageView(VkFormat _format)
{
	if(image_ == VK_NULL_HANDLE)
	{
		std::cout << "failed to create texture image view, VkImage not initialized!\n";
		return false;
	}

	VkImageViewCreateInfo viewInfo{};
	viewInfo.sType								= VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
	viewInfo.image								= image_;
	viewInfo.viewType							= VK_IMAGE_VIEW_TYPE_2D;
	viewInfo.format								= _format;
	viewInfo.subresourceRange.aspectMask		= VK_IMAGE_ASPECT_COLOR_BIT;
	viewInfo.subresourceRange.baseMipLevel		= 0;
	viewInfo.subresourceRange.levelCount		= 1;
	viewInfo.subresourceRange.baseArrayLayer	= 0;
	viewInfo.subresourceRange.layerCount		= 1;

	if (vkCreateImageView(logicalDevice_, &viewInfo, nullptr, &imageView_) != VK_SUCCESS)
	{
		std::cout<< "failed to create texture image view!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool Texture::CreateTextureSampler()
{
	VkPhysicalDeviceProperties properties{};
	vkGetPhysicalDeviceProperties(physicalDevice_, &properties);

	VkSamplerCreateInfo samplerInfo{};
	samplerInfo.sType					= VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
	samplerInfo.magFilter				= VK_FILTER_LINEAR;
	samplerInfo.minFilter				= VK_FILTER_LINEAR;
	samplerInfo.addressModeU			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeV			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.addressModeW			= VK_SAMPLER_ADDRESS_MODE_REPEAT;
	samplerInfo.anisotropyEnable		= VK_TRUE;
	samplerInfo.maxAnisotropy			= properties.limits.maxSamplerAnisotropy;
	samplerInfo.borderColor				= VK_BORDER_COLOR_INT_OPAQUE_BLACK;
	samplerInfo.unnormalizedCoordinates	= VK_FALSE;
	samplerInfo.compareEnable			= VK_FALSE;
	samplerInfo.compareOp				= VK_COMPARE_OP_ALWAYS;
	samplerInfo.mipmapMode				= VK_SAMPLER_MIPMAP_MODE_LINEAR;

	if (vkCreateSampler(logicalDevice_, &samplerInfo, nullptr, &sampler_) != VK_SUCCESS)
	{
		std::cout<< "failed to create texture sampler!\n";
		return false;
	}
	return true;
}

}