#pragma once

#include <glm/glm.hpp>
#include <vulkan/vulkan.h>
#include <memory>

namespace xengine
{

class GameObject
{
public:
	GameObject()			= default;
	virtual ~GameObject()	= default;

	void				SetPosition(const glm::vec3& _position)	{ position_ = _position; }
	const glm::vec3&	GetPosition()	const					{ return position_; }

	virtual void		UpdateUbo(const VkExtent2D&) = 0;

protected:
	glm::vec3 position_ = glm::vec3(0.0f);
};

}