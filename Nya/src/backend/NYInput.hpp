#pragma once
#include "pch.hpp"

namespace Nya {
	static class NYInput {
	public:
		NYInput();
		~NYInput();

		static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);
		static void mouse_button_callback(GLFWwindow* window, int button, int action, int mods);

		static bool isKeyPressed(int key) { return keyStates[key]; }
		static bool isButtonPressed(int button) { return buttonStates[button]; }
		static std::array<bool, 512> keyStates;
		static std::array<bool, 16> buttonStates;
	private:
	};
}
