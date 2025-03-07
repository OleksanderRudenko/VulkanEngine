#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace xengine
{

struct UniformBufferObject
{
	glm::mat4	model_;
	glm::mat4	view_;
	glm::mat4	proj_;
};

}