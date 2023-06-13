#include "pch.hpp"
#include "backend/NYRenderDevice.hpp"
#include "backend/NYWindow.hpp"
#include "backend/NYSwapchain.hpp"
#include "backend/NYPipeline.hpp"
#include "backend/NYRenderer.hpp"
#include "utils/NYTimer.hpp"
#include "game/NYSprite.hpp"
#include "systems/NYRenderingSystem.hpp"
#include "backend/NYTexture.hpp"
#include "backend/NYShader.hpp"
#include "backend/NYRenderpass.hpp"

namespace Nya {
	class Game {
		public:
			Game();
			~Game();

			void init();
			void update();

		NYWindow window{ 1280, 720, "testbed" };
	private:
		void initBackend();
		void makeRenderPasses();
		void initRendering();

	private:
		NYRenderDevice::NYRenderDeviceCreateInfo renderDeviceInfo{ "testbed", VK_MAKE_VERSION(1, 0, 0) };
		NYRenderDevice renderDevice{ renderDeviceInfo, window };
		NYSwapchain swapchain{ renderDevice };

		NYDescriptorSetLayout spriteLayout{ renderDevice };
		NYShader spriteShader{ renderDevice, "src/shaders/shader.vert", "src/shaders/shader.frag" };
		NYRenderPass renderPass{ renderDevice };
		std::unique_ptr<NYPipeline> pipeline;
		std::unique_ptr<NYRenderer> renderer;
		std::unique_ptr<NYRenderingSystem> renderingSystem;


		std::vector<std::unique_ptr<NYSprite>> sprites;
		std::vector<std::shared_ptr<NYTexture>> textures;
	};
}