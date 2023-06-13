#include "pch.hpp"
#include "game.hpp"


int main() {
	using namespace Nya;
	Game game;
	game.init();

	while (!glfwWindowShouldClose(game.window.getHandlePointer())) {
		game.update();
	}
}