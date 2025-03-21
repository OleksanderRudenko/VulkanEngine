#pragma once

#include "vulkan_engine_lib.h"
#include <GLFW/glfw3.h>
#include <functional>
#include <memory>
#include <string>

namespace xengine
{

class ENGINE_API Window final
{
public:
	Window(uint32_t				width,
		   uint32_t				height,
		   const std::string&	name);
	Window(const Window&)						= delete;
	Window(Window&&)							= delete;
	~Window();

	Window&		operator=(const Window&)		= delete;
	Window&		operator=(Window&&)				= delete;

	bool		Init();

	GLFWwindow	*GetWindow()			const	{ return window_.get(); }

	bool		IsResizable()			const	{ return isResizable_; }
	void		SetResizable(bool isResizable);
	bool		FramebufferResized()	const	{ return framebufferResized_; }
	void		FramebufferResizedReset()		{ framebufferResized_ = false; }

	uint32_t	Width()					const	{ return width_; }
	uint32_t	Height()				const	{ return height_; }

private:
	static void	FramebufferResizeCallback(GLFWwindow*	window,
										  int			width,
										  int			height);

	uint32_t	width_				= 0;
	uint32_t	height_				= 0;
	std::string	name_;

	std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>	window_;
	bool		isResizable_		= false;
	bool		framebufferResized_	= false;
};

}
