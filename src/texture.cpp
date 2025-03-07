#include "stdafx.h"
#include "texture.h"
#include "buffer.h"
#include <stb_image.h>
#include <iostream>

namespace xengine
{

//======================================================================================================================
Texture::Texture()
{}
//======================================================================================================================
bool Texture::Create(const std::string&					_path,
					 std::reference_wrapper<VkDevice>	_logicalDevice,
					 const VkPhysicalDevice&			_physicalDevice)
{
	int texWidth, texHeight, texChannels;
	stbi_uc* pixels = stbi_load("textures/texture.jpg", &texWidth, &texHeight, &texChannels, STBI_rgb_alpha);
	VkDeviceSize imageSize = texWidth * texHeight * 4;

	if (!pixels)
	{
		std::cout<< "failed to load texture image!\n";
		return false;
	}

	Buffer stagingBuffer(imageSize, std::ref(_logicalDevice));
	stagingBuffer.CreateBuffer(_physicalDevice,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);
	void* data;
	vkMapMemory(_logicalDevice, stagingBuffer.GetBufferMemory(), 0, imageSize, 0, &data);
	memcpy(data, pixels, static_cast<size_t>(imageSize));
	vkUnmapMemory(_logicalDevice, stagingBuffer.GetBufferMemory());
	stbi_image_free(pixels);

	VkImageCreateInfo imageInfo{};
	imageInfo.sType			= VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO;
	imageInfo.imageType		= VK_IMAGE_TYPE_2D;
	imageInfo.extent.width	= texWidth;
	imageInfo.extent.height	= texHeight;
	imageInfo.extent.depth	= 1;
	imageInfo.mipLevels		= 1;
	imageInfo.arrayLayers	= 1;
	imageInfo.format		= VK_FORMAT_R8G8B8A8_SRGB;
	imageInfo.tiling		= VK_IMAGE_TILING_OPTIMAL;
	imageInfo.initialLayout	= VK_IMAGE_LAYOUT_UNDEFINED;
	imageInfo.usage			= VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT;
	imageInfo.samples		= VK_SAMPLE_COUNT_1_BIT;
	imageInfo.sharingMode	= VK_SHARING_MODE_EXCLUSIVE;

	if (vkCreateImage(_logicalDevice, &imageInfo, nullptr, &image_) != VK_SUCCESS)
	{
		std::cout<< "failed to create image!\n";
		return false;
	}

	VkMemoryRequirements memRequirements;
	vkGetImageMemoryRequirements(_logicalDevice, image_, &memRequirements);

	VkMemoryAllocateInfo allocInfo{};
	allocInfo.sType				= VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO;
	allocInfo.allocationSize	= memRequirements.size;
	allocInfo.memoryTypeIndex	= Buffer::FindMemoryType(_physicalDevice, memRequirements.memoryTypeBits, VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);

	if (vkAllocateMemory(_logicalDevice, &allocInfo, nullptr, &imageMemory_) != VK_SUCCESS)
	{
		std::cout<< "failed to allocate image memory!\n";
		return false;
	}

	vkBindImageMemory(_logicalDevice, image_, imageMemory_, 0);

	return true;
}

}