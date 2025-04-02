#include "stdafx.h"
#include "sprite.h"
#include "buffer.h"
#include "command_buffer.h"
#include "texture.h"
#include "tools.h"
#include "vertex.h"
#include "tools/timer.h"
#include "window.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Sprite::Sprite(std::reference_wrapper<VkDevice>			_logicalDevice,
			   std::reference_wrapper<VkPhysicalDevice>	_physicalDevice,
			   std::shared_ptr<Window>					_window,
			   const QueueFamilyIndices&				_queueFamilyIndices)
: logicalDevice_(_logicalDevice)
, physicalDevice_(_physicalDevice)
, window_(_window)
, queueFamilyIndices_(_queueFamilyIndices)
{}
//======================================================================================================================
Sprite::~Sprite()
{
	vertexBuffer_.reset();
	indexBuffer_.reset();
	uniformBuffer_.reset();
	texture_.reset();
	vkDestroyDescriptorPool(logicalDevice_, descriptorPool_, nullptr);
}
//======================================================================================================================
bool Sprite::Create(const std::string&				_texturePath,
					std::shared_ptr<CommandPool>	_commandPool,
					VkDescriptorSetLayout			_descriptorSetLayout,
					VkQueue							_graphicsQueue)
{
	texture_ = std::make_unique<Texture>(logicalDevice_,
										 physicalDevice_,
										 queueFamilyIndices_);
	if (!texture_->Create(_texturePath))
	{
		return false;
	}

	if(!texture_.get()->TransitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
											  VK_IMAGE_LAYOUT_UNDEFINED,
											  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
											  _commandPool,
											  _graphicsQueue))
	{
		return false;
	}
	texture_.get()->CopyBufferToImage(_commandPool, _graphicsQueue);

	// To be able to start sampling from the texture image in the shader
	texture_.get()->TransitionImageLayout(VK_FORMAT_R8G8B8A8_SRGB,
										  VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
										  VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
										  _commandPool,
										  _graphicsQueue);
	texture_.get()->CreateTextureImageView();
	texture_.get()->CreateTextureSampler();
	CreateUniformBuffer();
	CreateDescriptorPool();
	CreateDescriptorSet(_descriptorSetLayout);
	CreateVertexBuffer(_commandPool, _graphicsQueue);
	CreateIndexBuffer(_commandPool, _graphicsQueue);
	return true;
}
//======================================================================================================================
void Sprite::UpdateUbo(const VkExtent2D& _extent)
{
	float aspectRatio = (float)_extent.width / (float)_extent.height;
	ubo_.model	= glm::translate(glm::mat4(1.0f), position_);
	ubo_.view	= glm::lookAt(glm::vec3(1.0f, 0.0f, 4.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, -1.0f, 0.0f));
	float zoomFactor = 0.5f;
	ubo_.proj = glm::ortho(-aspectRatio / zoomFactor, aspectRatio / zoomFactor, -1.0f / zoomFactor, 1.0f / zoomFactor, 0.1f, 10.0f);
	ubo_.proj[1][1] *= -1;
	memcpy(uniformBufferMapped_, &ubo_, sizeof(ubo_));

	//ubo_.proj	= glm::perspective(glm::radians(60.0f), window_->Width() / (float)window_->Height(), 0.1f, 10.0f);
	//ubo_.model = glm::translate(glm::mat4(1.0f), position_) * glm::scale(glm::mat4(1.0f), glm::vec3(texture_->GetWidth(), texture_->GetHeight(), 1.0f));
	//ubo_.view	= glm::mat4(1.0f);
}
//======================================================================================================================
bool Sprite::CreateDescriptorPool()
{
	std::array<VkDescriptorPoolSize, 2> poolSizes{};
	poolSizes[0].type				= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	poolSizes[0].descriptorCount	= 1;  // Only one per sprite
	poolSizes[1].type				= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	poolSizes[1].descriptorCount	= 1;  // Only one per sprite

	VkDescriptorPoolCreateInfo poolInfo{};
	poolInfo.sType         = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
	poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
	poolInfo.pPoolSizes    = poolSizes.data();
	poolInfo.maxSets       = 1;  // One set per sprite

	if (vkCreateDescriptorPool(logicalDevice_, &poolInfo, nullptr, &descriptorPool_) != VK_SUCCESS)
	{
		std::cout << "failed to create descriptor pool!\n";
		return false;
	}
	return true;
}
//======================================================================================================================
bool Sprite::CreateDescriptorSet(VkDescriptorSetLayout	_descriptorSetLayout)
{
	VkDescriptorSetAllocateInfo allocInfo{};
	allocInfo.sType					= VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO;
	allocInfo.descriptorPool		= descriptorPool_;
	allocInfo.descriptorSetCount	= 1;
	allocInfo.pSetLayouts			= &_descriptorSetLayout;

	VkResult result = vkAllocateDescriptorSets(logicalDevice_.get(), &allocInfo, &descriptorSet_);
	if (result != VK_SUCCESS)
	{
		std::cout << "failed to create descriptor set!\n";
		return false;
	}

	VkDescriptorBufferInfo bufferInfo{};
	bufferInfo.buffer = uniformBuffer_->GetBuffer();
	bufferInfo.offset = 0;
	bufferInfo.range = sizeof(UniformBufferObject);

	VkDescriptorImageInfo imageInfo{};
	imageInfo.imageLayout	= VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
	imageInfo.imageView		= texture_->GetImageView();
	imageInfo.sampler		= texture_->GetSampler();

	std::array<VkWriteDescriptorSet, 2> descriptorWrites{};

	descriptorWrites[0].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[0].dstSet			= descriptorSet_;
	descriptorWrites[0].dstBinding		= 0;
	descriptorWrites[0].dstArrayElement	= 0;
	descriptorWrites[0].descriptorType	= VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
	descriptorWrites[0].descriptorCount	= 1;
	descriptorWrites[0].pBufferInfo		= &bufferInfo;

	descriptorWrites[1].sType			= VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
	descriptorWrites[1].dstSet			= descriptorSet_;
	descriptorWrites[1].dstBinding		= 1;
	descriptorWrites[1].dstArrayElement	= 0;
	descriptorWrites[1].descriptorType	= VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
	descriptorWrites[1].descriptorCount	= 1;
	descriptorWrites[1].pImageInfo		= &imageInfo;

	vkUpdateDescriptorSets(logicalDevice_.get(), static_cast<uint32_t>(descriptorWrites.size()), descriptorWrites.data(), 0, nullptr);
	return true;
}
//======================================================================================================================
void Sprite::CreateVertexBuffer(std::shared_ptr<CommandPool>	_commandPool,
								VkQueue							_graphicsQueue)
{
	Buffer stagingBuffer(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	stagingBuffer.CreateBuffer(physicalDevice_,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), 0, stagingBuffer.GetSize(), 0, &data);
	memcpy(data, vertices.data(), (size_t)stagingBuffer.GetSize());
	vkUnmapMemory(logicalDevice_, stagingBuffer.GetBufferMemory());

	vertexBuffer_ = std::make_unique<Buffer>(sizeof(vertices[0]) * vertices.size(), std::ref(logicalDevice_));
	vertexBuffer_.get()->CreateBuffer(physicalDevice_,
									  VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
									  VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CommandBuffer commandBuffer(std::ref(logicalDevice_), std::ref(physicalDevice_), queueFamilyIndices_);
	commandBuffer.Create(_commandPool);
	commandBuffer.CopyBuffer(stagingBuffer.GetBuffer(), vertexBuffer_->GetBuffer(), vertexBuffer_->GetSize());
	commandBuffer.SubmitAndWaitIdle(_graphicsQueue);
}
//======================================================================================================================
void Sprite::CreateIndexBuffer(std::shared_ptr<CommandPool>		_commandPool,
							   VkQueue							_graphicsQueue)
{
	Buffer stagingBuffer(sizeof(indices[0]) * indices.size(), std::ref(logicalDevice_));
	stagingBuffer.CreateBuffer(physicalDevice_,
							   VK_BUFFER_USAGE_TRANSFER_SRC_BIT,
							   VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	void* data;
	vkMapMemory(logicalDevice_, stagingBuffer.GetBufferMemory(), 0, stagingBuffer.GetSize(), 0, &data);
	memcpy(data, indices.data(), (size_t)stagingBuffer.GetSize());
	vkUnmapMemory(logicalDevice_, stagingBuffer.GetBufferMemory());

	indexBuffer_ = std::make_unique<Buffer>(sizeof(indices[0]) * indices.size(), std::ref(logicalDevice_));
	indexBuffer_.get()->CreateBuffer(physicalDevice_,
									 VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_INDEX_BUFFER_BIT,
									 VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT);
	CommandBuffer commandBuffer(std::ref(logicalDevice_), std::ref(physicalDevice_), queueFamilyIndices_);
	commandBuffer.Create(_commandPool);
	commandBuffer.CopyBuffer(stagingBuffer.GetBuffer(), indexBuffer_->GetBuffer(), indexBuffer_->GetSize());
	commandBuffer.SubmitAndWaitIdle(_graphicsQueue);
}
//======================================================================================================================
void Sprite::CreateUniformBuffer()
{
	VkDeviceSize bufferSize = sizeof(UniformBufferObject);

	uniformBuffer_ = std::make_unique<Buffer>(bufferSize, std::ref(logicalDevice_));
	uniformBuffer_->CreateBuffer(physicalDevice_,
								 VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
								 VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

	vkMapMemory(logicalDevice_, uniformBuffer_->GetBufferMemory(), 0, bufferSize, 0, &uniformBufferMapped_);
}

}