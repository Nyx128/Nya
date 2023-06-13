#include "pch.hpp"
#include "NYInput.hpp"

namespace Nya {
	std::array<bool, 512> NYInput::keyStates;
	std::array<bool, 16> NYInput::buttonStates;
	NYInput::NYInput(){ 
		for (int i = 0; i < NYInput::keyStates.size(); i++) {
			NYInput::keyStates[i] = false;
		}

		for (int i = 0; i < NYInput::buttonStates.size(); i++) {
			NYInput::buttonStates[i] = false;
		}
	}
	NYInput::~NYInput()
	{
	}
	void NYInput::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods){
		if (action == GLFW_PRESS) {
			NYInput::keyStates[key] = true;
		}
		if (action == GLFW_RELEASE) {
			NYInput::keyStates[key] = false;
		}
	}

	void NYInput::mouse_button_callback(GLFWwindow* window, int button, int action, int mods){
		if (action == GLFW_PRESS) {
			NYInput::buttonStates[button] = true;
		}
		if (action == GLFW_RELEASE) {
			NYInput::buttonStates[button] = false;
		}
	}
}