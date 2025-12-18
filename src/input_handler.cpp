#include "stdafx.h"
#include "input_handler.h"
#include "window.h"
#include <iostream>

namespace xengine
{

//======================================================================================================================
InputHandler::InputHandler()
{
	keyStates_.fill(KeyState::Released);
	previousKeyStates_.fill(KeyState::Released);
	mouseButtonStates_.fill(KeyState::Released);
	previousMouseButtonStates_.fill(KeyState::Released);
}
//======================================================================================================================
void InputHandler::Init(GLFWwindow* _window)
{
	window_ = _window;

	WindowUserPointer* userPtr = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(window_));
	if (userPtr)
	{
		userPtr->inputHandler = this;
	}

	// Register GLFW callbacks
	glfwSetKeyCallback(window_, KeyCallbackStatic);
	glfwSetMouseButtonCallback(window_, MouseButtonCallbackStatic);
	glfwSetCursorPosCallback(window_, CursorPositionCallbackStatic);
	glfwSetScrollCallback(window_, ScrollCallbackStatic);
}
//======================================================================================================================
void InputHandler::Update()
{
	// Update key state transitions
	UpdateKeyStates();
	UpdateMouseButtonStates();

	// Reset frame-specific data
	mouseDelta_ = glm::vec2(0.0f);
	scrollDelta_ = 0.0f;
}
//======================================================================================================================
void InputHandler::UpdateKeyStates()
{
	for (int i = 0; i < MAX_KEYS; ++i)
	{
		KeyState currentState = keyStates_[i];
		KeyState previousState = previousKeyStates_[i];

		if (currentState == KeyState::Pressed)
		{
			// Pressed becomes Held on the next frame
			previousKeyStates_[i] = KeyState::Held;
		}
		else if (currentState == KeyState::JustReleased)
		{
			// JustReleased becomes Released on the next frame
			keyStates_[i] = KeyState::Released;
			previousKeyStates_[i] = KeyState::Released;
		}
		else if (currentState == KeyState::Held)
		{
			previousKeyStates_[i] = KeyState::Held;
		}
	}
}
//======================================================================================================================
void InputHandler::UpdateMouseButtonStates()
{
	for (int i = 0; i < MAX_MOUSE_BUTTONS; ++i)
	{
		KeyState currentState = mouseButtonStates_[i];

		if (currentState == KeyState::Pressed)
		{
			previousMouseButtonStates_[i] = KeyState::Held;
		}
		else if (currentState == KeyState::JustReleased)
		{
			mouseButtonStates_[i] = KeyState::Released;
			previousMouseButtonStates_[i] = KeyState::Released;
		}
		else if (currentState == KeyState::Held)
		{
			previousMouseButtonStates_[i] = KeyState::Held;
		}
	}
}
//======================================================================================================================
bool InputHandler::IsKeyPressed(int _key) const
{
	if(_key < 0 || _key >= MAX_KEYS)
	{
		return false;
	}
	return keyStates_[_key] == KeyState::Pressed;
}
//======================================================================================================================
bool InputHandler::IsKeyHeld(int _key) const
{
	if(_key < 0 || _key >= MAX_KEYS)
	{
		return false;
	}
	return keyStates_[_key] == KeyState::Held;
}
//======================================================================================================================
bool InputHandler::IsKeyReleased(int _key) const
{
	if(_key < 0 || _key >= MAX_KEYS)
	{
		return false;
	}
	return keyStates_[_key] == KeyState::JustReleased;
}
//======================================================================================================================
bool InputHandler::IsKeyDown(int _key) const
{
	if(_key < 0 || _key >= MAX_KEYS)
	{
		return false;
	}
	return keyStates_[_key] == KeyState::Pressed || keyStates_[_key] == KeyState::Held;
}
//======================================================================================================================
bool InputHandler::IsShiftPressed() const
{
	return IsKeyDown(GLFW_KEY_LEFT_SHIFT) || IsKeyDown(GLFW_KEY_RIGHT_SHIFT);
}
//======================================================================================================================
bool InputHandler::IsCtrlPressed() const
{
	return IsKeyDown(GLFW_KEY_LEFT_CONTROL) || IsKeyDown(GLFW_KEY_RIGHT_CONTROL);
}
//======================================================================================================================
bool InputHandler::IsAltPressed() const
{
	return IsKeyDown(GLFW_KEY_LEFT_ALT) || IsKeyDown(GLFW_KEY_RIGHT_ALT);
}
//======================================================================================================================
bool InputHandler::IsMouseButtonPressed(int _button) const
{
	if(_button < 0 || _button >= MAX_MOUSE_BUTTONS)
	{
		return false;
	}
	return mouseButtonStates_[_button] == KeyState::Pressed;
}
//======================================================================================================================
bool InputHandler::IsMouseButtonHeld(int _button) const
{
	if(_button < 0 || _button >= MAX_MOUSE_BUTTONS)
	{
		return false;
	}
	return mouseButtonStates_[_button] == KeyState::Held;
}
//======================================================================================================================
bool InputHandler::IsMouseButtonReleased(int _button) const
{
	if(_button < 0 || _button >= MAX_MOUSE_BUTTONS)
	{
		return false;
	}
	return mouseButtonStates_[_button] == KeyState::JustReleased;
}
//======================================================================================================================
bool InputHandler::IsMouseButtonDown(int _button) const
{
	if(_button < 0 || _button >= MAX_MOUSE_BUTTONS)
	{
		return false;
	}
	return mouseButtonStates_[_button] == KeyState::Pressed || mouseButtonStates_[_button] == KeyState::Held;
}
//======================================================================================================================
void InputHandler::MapAction(const std::string& _action, int _key)
{
	actionMap_[_action] = _key;
}
//======================================================================================================================
void InputHandler::UnmapAction(const std::string& _action)
{
	actionMap_.erase(_action);
}
//======================================================================================================================
bool InputHandler::IsActionPressed(const std::string& _action) const
{
	auto it = actionMap_.find(_action);
	if(it == actionMap_.end())
	{
		return false;
	}
	return IsKeyPressed(it->second);
}
//======================================================================================================================
bool InputHandler::IsActionHeld(const std::string& _action) const
{
	auto it = actionMap_.find(_action);
	if(it == actionMap_.end())
	{
		return false;
	}
	return IsKeyHeld(it->second);
}
//======================================================================================================================
bool InputHandler::IsActionReleased(const std::string& _action) const
{
	auto it = actionMap_.find(_action);
	if(it == actionMap_.end())
	{
		return false;
	}
	return IsKeyReleased(it->second);
}
//======================================================================================================================
void InputHandler::RegisterKeyCallback(const std::string& _name, KeyCallback _callback)
{
	keyCallbacks_[_name] = _callback;
}
//======================================================================================================================
void InputHandler::RegisterMouseButtonCallback(const std::string& _name, MouseButtonCallback _callback)
{
	mouseButtonCallbacks_[_name] = _callback;
}
//======================================================================================================================
void InputHandler::RegisterScrollCallback(const std::string& _name, ScrollCallback _callback)
{
	scrollCallbacks_[_name] = _callback;
}
//======================================================================================================================
void InputHandler::UnregisterKeyCallback(const std::string& _name)
{
	keyCallbacks_.erase(_name);
}
//======================================================================================================================
void InputHandler::UnregisterMouseButtonCallback(const std::string& _name)
{
	mouseButtonCallbacks_.erase(_name);
}
//======================================================================================================================
void InputHandler::UnregisterScrollCallback(const std::string& _name)
{
	scrollCallbacks_.erase(_name);
}
//======================================================================================================================
void InputHandler::KeyCallbackStatic(GLFWwindow* _window, int _key, int _scancode, int _action, int _mods)
{
	WindowUserPointer* userPtr = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(_window));
	if (userPtr && userPtr->inputHandler)
	{
		userPtr->inputHandler->OnKey(_key, _scancode, _action, _mods);
	}
}
//======================================================================================================================
void InputHandler::MouseButtonCallbackStatic(GLFWwindow* _window, int _button, int _action, int _mods)
{
	WindowUserPointer* userPtr = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(_window));
	if (userPtr && userPtr->inputHandler)
	{
		userPtr->inputHandler->OnMouseButton(_button, _action, _mods);
	}
}
//======================================================================================================================
void InputHandler::CursorPositionCallbackStatic(GLFWwindow* _window, double _xpos, double _ypos)
{
	WindowUserPointer* userPtr = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(_window));
	if (userPtr && userPtr->inputHandler)
	{
		userPtr->inputHandler->OnCursorPosition(_xpos, _ypos);
	}
}
//======================================================================================================================
void InputHandler::ScrollCallbackStatic(GLFWwindow* _window, double _xoffset, double _yoffset)
{
	WindowUserPointer* userPtr = static_cast<WindowUserPointer*>(glfwGetWindowUserPointer(_window));
	if (userPtr && userPtr->inputHandler)
	{
		userPtr->inputHandler->OnScroll(_xoffset, _yoffset);
	}
}
//======================================================================================================================
void InputHandler::OnKey(int _key, int _scancode, int _action, int _mods)
{
	if (_key < 0 || _key >= MAX_KEYS)
		return;

	// Update key state based on GLFW action
	if (_action == GLFW_PRESS)
	{
		keyStates_[_key] = KeyState::Pressed;
	}
	else if (_action == GLFW_RELEASE)
	{
		keyStates_[_key] = KeyState::JustReleased;
	}
	// GLFW_REPEAT is ignored - we handle held state in Update()

	// Call registered callbacks
	for (auto& callback : keyCallbacks_)
	{
		callback.second(_key, _action, _mods);
	}
}
//======================================================================================================================
void InputHandler::OnMouseButton(int _button, int _action, int _mods)
{
	if (_button < 0 || _button >= MAX_MOUSE_BUTTONS)
		return;

	if (_action == GLFW_PRESS)
	{
		mouseButtonStates_[_button] = KeyState::Pressed;
	}
	else if (_action == GLFW_RELEASE)
	{
		mouseButtonStates_[_button] = KeyState::JustReleased;
	}

	// Call registered callbacks
	for (auto& callback : mouseButtonCallbacks_)
	{
		callback.second(_button, _action, _mods);
	}
}
//======================================================================================================================
void InputHandler::OnCursorPosition(double _xpos, double _ypos)
{
	glm::vec2 newPosition(_xpos, _ypos);

	if (firstMouseInput_)
	{
		previousMousePosition_ = newPosition;
		firstMouseInput_ = false;
	}

	mouseDelta_ = newPosition - previousMousePosition_;
	mousePosition_ = newPosition;
	previousMousePosition_ = newPosition;
}
//======================================================================================================================
void InputHandler::OnScroll(double _xoffset, double _yoffset)
{
	scrollDelta_ = static_cast<float>(_yoffset);

	// Call registered callbacks
	for (auto& callback : scrollCallbacks_)
	{
		callback.second(scrollDelta_);
	}
}

}
