#include "stdafx.h"
#include "window.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
Window::Window(uint32_t				_width,
			   uint32_t				_height,
			   const std::string&	_name)
: width_(_width)
, height_(_height)
, name_(_name)
{}
//======================================================================================================================
Window::~Window()
{
	glfwTerminate();
}
//======================================================================================================================
bool Window::Init()
{
	if(glfwInit() == GLFW_FALSE)
	{
		std::cout << "glfwInit failed\n";
		return false;
	}

	if(glfwVulkanSupported() == GLFW_FALSE)
	{
		std::cout << "Vulkan not supported\n";
		return false;
	}

	glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);

	window_ = std::unique_ptr<GLFWwindow, std::function<void(GLFWwindow*)>>(glfwCreateWindow(width_, height_, "Vulkan", nullptr, nullptr),
																			[](GLFWwindow* window) { if (window) glfwDestroyWindow(window);});

	glfwSetWindowUserPointer(window_.get(), this);
	glfwSetFramebufferSizeCallback(window_.get(), FramebufferResizeCallback);

	return true;
}
//======================================================================================================================
void Window::SetResizable(bool _isResizable)
{
	isResizable_ = _isResizable;
	glfwSetWindowAttrib(window_.get(), GLFW_RESIZABLE, isResizable_);
}
//======================================================================================================================
void Window::FramebufferResizeCallback(GLFWwindow*	_window,
									   int			_width,
									   int			_height)
{
	Window* instance = static_cast<Window*>(glfwGetWindowUserPointer(_window));
	if(instance)
	{
		instance->framebufferResized_	= true;
		instance->width_				= _width;
		instance->height_				= _height;
	}
}

}
