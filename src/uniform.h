#pragma once

#define GLM_FORCE_RADIANS
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

namespace xengine
{

struct UniformBufferObject
{
	glm::mat4	model;
	glm::mat4	view;
	glm::mat4	proj;
};

}