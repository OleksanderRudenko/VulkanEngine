#pragma once

#include "uniform.h"
#include "vulkan_engine_lib.h"
#include <vulkan/vulkan.h>
#include <functional>
#include <memory>
#include <string>

namespace xengine
{

class Texture;
struct QueueFamilyIndices;
struct Vertex;
class CommandPool;
class Buffer;
class Window;

class Sprite
{
public:
	Sprite(std::reference_wrapper<VkDevice> logicalDevice,
		   std::reference_wrapper<VkPhysicalDevice>,
		   std::shared_ptr<Window>,
		   const QueueFamilyIndices&);
	Sprite(const Sprite&)				= default;
	Sprite(Sprite&&)					= default;
	virtual ~Sprite();

	Sprite& operator=(const Sprite&)	= default;
	Sprite& operator=(Sprite&&)			= default;

	bool					Create(const std::string& texturePath,
								   std::shared_ptr<CommandPool>,
								   VkDescriptorPool,
								   VkDescriptorSetLayout,
								   VkQueue);

	void					UpdateUbo();
	const VkDescriptorSet&	GetDescriptorSet()	const { return descriptorSet_; }
	const Buffer*			GetVertexBuffer()	const { return vertexBuffer_.get(); }
	const Buffer*			GetIndexBuffer()	const { return indexBuffer_.get(); }
	const Texture*			GetTexture()		const { return texture_.get(); }

	void					SetPosition(glm::vec3 _position) { position_ = _position; }

private:
	bool					CreateDescriptorSet(VkDescriptorPool, VkDescriptorSetLayout);
	void					CreateVertexBuffer(std::shared_ptr<CommandPool>,
											   VkQueue graphicsQueue);
	void					CreateIndexBuffer(std::shared_ptr<CommandPool>,
											  VkQueue graphicsQueue);
	void					CreateUniformBuffer();

	const std::reference_wrapper<VkDevice>			logicalDevice_;
	const std::reference_wrapper<VkPhysicalDevice>	physicalDevice_;
	std::shared_ptr<Window>							window_;
	const QueueFamilyIndices&						queueFamilyIndices_;

	std::unique_ptr<Texture>						texture_;
	VkDescriptorSet									descriptorSet_;
	std::unique_ptr<Buffer>							vertexBuffer_;
	std::unique_ptr<Buffer>							indexBuffer_;
	std::unique_ptr<Buffer>							uniformBuffer_;
	void*											uniformBufferMapped_;

	UniformBufferObject								ubo_	= {};

	glm::vec3										position_ = glm::vec3(0.0f);

};

}