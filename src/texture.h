#pragma once

#include <vulkan/vulkan.h>
#include <string>

class Texture
{
public:
	Texture();

	Texture(const Texture&)					= default;
	Texture(Texture&&)						= default;
	virtual ~Texture()						= default;

	Texture&	operator=(const Texture&)	= default;
	Texture&	operator=(Texture&&)		= default;

	bool		Create(const std::string&				path,
					   std::reference_wrapper<VkDevice>	logicalDevice,
					   const VkPhysicalDevice&);

protected:
	VkImage			image_;
	VkDeviceMemory	imageMemory_;
};