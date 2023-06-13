#include "pch.hpp"
#include "NYWindow.hpp"
#include "logging/NYLogger.hpp"
#include "NYInput.hpp"

//its pretty simple and nothing much to it
namespace Nya {
	NYWindow::NYWindow(uint32_t _width, uint32_t _height, const char* _title):width(_width), height(_height), title(_title) {
		//TODO:resizing
		glfwInit();
		glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
		glfwWindowHint(GLFW_RESIZABLE, GLFW_FALSE);\
		window = glfwCreateWindow(width, height, title, NULL, NULL);
		glfwSetKeyCallback(window, NYInput::key_callback);
		glfwSetMouseButtonCallback(window, NYInput::mouse_button_callback);
		NYLogger::logTrace("NYWindow created");
	}

	NYWindow::~NYWindow(){
		glfwDestroyWindow(window);
		glfwTerminate();
		NYLogger::logTrace("NYWindow destroyed");

	}
}