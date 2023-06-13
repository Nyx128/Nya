#pragma once
//abstraction for window creation using glfw
#include "pch.hpp"

namespace Nya {
	class NYWindow {
	public:
		//window will be initizlized with its dimensions and title
		NYWindow(uint32_t _width, uint32_t _height, const char* _title);
		~NYWindow();

		NYWindow(NYWindow const&) = delete;
		NYWindow& operator=(NYWindow const&) = delete;

		inline GLFWwindow* getHandlePointer() { return window; }
	private:
		GLFWwindow* window;
		uint32_t width;
		uint32_t height;
		const char* title;
	};
}
