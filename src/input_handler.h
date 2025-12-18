#pragma once

#include "vulkan_engine_lib.h"
#include <GLFW/glfw3.h>
#include <glm/glm.hpp>
#include <array>
#include <unordered_map>
#include <string>
#include <functional>

// Polling-based (in game loop)
//if (inputHandler_->IsKeyPressed(GLFW_KEY_SPACE)) {
//    player->Jump();
//}
//
//if (inputHandler_->IsKeyHeld(GLFW_KEY_W)) {
//    player->MoveForward(deltaTime);
//}
//
//// Mouse input
//glm::vec2 mouseDelta = inputHandler_->GetMouseDelta();
//camera->Rotate(mouseDelta.x, mouseDelta.y);
//
//// Action mapping
//inputHandler_->MapAction("Jump", GLFW_KEY_SPACE);
//if (inputHandler_->IsActionPressed("Jump")) {
//    player->Jump();
//}
//
//// Event callbacks (for UI or one-time events)
//inputHandler_->RegisterKeyCallback("esc_handler", [](int key, int action, int mods) {
//    if (key == GLFW_KEY_ESCAPE && action == GLFW_PRESS) {
//        // Open pause menu
//    }
//});

namespace xengine
{

enum class KeyState
{
	Released,
	Pressed,
	Held,
	JustReleased
};

class ENGINE_API InputHandler
{
public:
	InputHandler();
	InputHandler(const InputHandler&)			= delete;
	InputHandler(InputHandler&&)				= delete;
	~InputHandler()								= default;

	InputHandler&	operator=(const InputHandler&)	= delete;
	InputHandler&	operator=(InputHandler&&)		= delete;

	void			Init(GLFWwindow* window);

	void			Update();

	bool			IsKeyPressed(int key)		const;	// True on the frame the key was pressed
	bool			IsKeyHeld(int key)			const;	// True while key is held down
	bool			IsKeyReleased(int key)		const;	// True on the frame the key was released
	bool			IsKeyDown(int key)			const;	// True if key is currently down (Pressed or Held)

	bool			IsShiftPressed()			const;
	bool			IsCtrlPressed()				const;
	bool			IsAltPressed()				const;

	bool			IsMouseButtonPressed(int button)	const;
	bool			IsMouseButtonHeld(int button)		const;
	bool			IsMouseButtonReleased(int button)	const;
	bool			IsMouseButtonDown(int button)		const;

	glm::vec2		GetMousePosition()			const	{ return mousePosition_; }
	glm::vec2		GetMouseDelta()				const	{ return mouseDelta_; }
	float			GetScrollDelta()			const	{ return scrollDelta_; }

	void			MapAction(const std::string& action, int key);
	void			UnmapAction(const std::string& action);
	bool			IsActionPressed(const std::string& action)		const;
	bool			IsActionHeld(const std::string& action)			const;
	bool			IsActionReleased(const std::string& action)		const;

	// Event callbacks - Register callbacks for specific events
	using KeyCallback = std::function<void(int key, int action, int mods)>;
	using MouseButtonCallback = std::function<void(int button, int action, int mods)>;
	using ScrollCallback = std::function<void(float delta)>;

	void			RegisterKeyCallback(const std::string& name, KeyCallback callback);
	void			RegisterMouseButtonCallback(const std::string& name, MouseButtonCallback callback);
	void			RegisterScrollCallback(const std::string& name, ScrollCallback callback);

	void			UnregisterKeyCallback(const std::string& name);
	void			UnregisterMouseButtonCallback(const std::string& name);
	void			UnregisterScrollCallback(const std::string& name);

private:
	// GLFW static callbacks
	static void		KeyCallbackStatic(GLFWwindow* window, int key, int scancode, int action, int mods);
	static void		MouseButtonCallbackStatic(GLFWwindow* window, int button, int action, int mods);
	static void		CursorPositionCallbackStatic(GLFWwindow* window, double xpos, double ypos);
	static void		ScrollCallbackStatic(GLFWwindow* window, double xoffset, double yoffset);

	// Instance callbacks
	void			OnKey(int key, int scancode, int action, int mods);
	void			OnMouseButton(int button, int action, int mods);
	void			OnCursorPosition(double xpos, double ypos);
	void			OnScroll(double xoffset, double yoffset);

	// Update key state transitions
	void			UpdateKeyStates();
	void			UpdateMouseButtonStates();

	GLFWwindow*		window_							= nullptr;

	// Keyboard state (using GLFW key codes, max is GLFW_KEY_LAST which is 348)
	static constexpr int MAX_KEYS = 512;
	std::array<KeyState, MAX_KEYS>					keyStates_;
	std::array<KeyState, MAX_KEYS>					previousKeyStates_;

	// Mouse state (GLFW supports up to 8 mouse buttons)
	static constexpr int MAX_MOUSE_BUTTONS = 8;
	std::array<KeyState, MAX_MOUSE_BUTTONS>			mouseButtonStates_;
	std::array<KeyState, MAX_MOUSE_BUTTONS>			previousMouseButtonStates_;

	// Mouse position and movement
	glm::vec2		mousePosition_					= glm::vec2(0.0f);
	glm::vec2		previousMousePosition_			= glm::vec2(0.0f);
	glm::vec2		mouseDelta_						= glm::vec2(0.0f);
	float			scrollDelta_					= 0.0f;
	bool			firstMouseInput_				= true;

	// Action mapping (action name -> key code)
	std::unordered_map<std::string, int>			actionMap_;

	// Event callbacks
	std::unordered_map<std::string, KeyCallback>			keyCallbacks_;
	std::unordered_map<std::string, MouseButtonCallback>	mouseButtonCallbacks_;
	std::unordered_map<std::string, ScrollCallback>			scrollCallbacks_;
};

}
