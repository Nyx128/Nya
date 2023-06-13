#include "pch.hpp"
#include "game.hpp"
#include "backend/NYInput.hpp"

namespace Nya {
	Game::Game() { initBackend();};
	Game::~Game() {
		renderDevice.getDevice().waitIdle();
	};

	void Game::initBackend() {
		NYPipelineConfig pipelineConfig;
		auto bindingDesc = NYSprite::getBindingDescriptions();
		auto attribDesc = NYSprite::getAttributeDescriptions();

		spriteLayout.addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics);
		spriteLayout.addBinding(vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics);
		spriteLayout.buildLayout();

		NYPipeline::createDefaultPipelineConfig(pipelineConfig, swapchain, bindingDesc, attribDesc);

		makeRenderPasses();

		pipeline = std::make_unique<NYPipeline>(renderDevice, pipelineConfig, spriteShader, spriteLayout, renderPass);
		swapchain.createFrameBuffers(renderPass.getRenderpass());

		renderer = std::make_unique<NYRenderer>(renderDevice, swapchain);
	}

	void Game::makeRenderPasses() {
		renderPass.addAttachment(swapchain.getSwapchainFormat(), vk::AttachmentLoadOp::eClear, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eUndefined, vk::ImageLayout::eColorAttachmentOptimal);

		//make a reference of the attachment to be used in the subpass
		vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		std::vector<vk::AttachmentReference> colorAttachmentRefs;
		colorAttachmentRefs.push_back(colorAttachmentRef);

		std::vector<vk::AttachmentReference> inputAttachmentRefs;

		renderPass.addSubpass(colorAttachmentRefs, inputAttachmentRefs, nullptr);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency.setDstSubpass(0);
		dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		renderPass.addDependency(dependency);

		renderPass.build();
	}

	void Game::init() {
		textures.resize(2);
		textures[0] = std::make_shared<NYTexture>(renderDevice, "res/1K-wood_plank_14_Dif.jpg");
		textures[1] = std::make_shared<NYTexture>(renderDevice, "res/zoro_dressrosa_drip_black.png");

		sprites.resize(2);
		sprites[0] = std::make_unique<NYSprite>(renderDevice);

		sprites[0]->translation = glm::vec3(2.0f, 0.0f, 0.0f);
		sprites[0]->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		sprites[0]->scale = glm::vec3(2.0f, 2.0f, 1.0f);
		sprites[0]->writeTexture(textures[0]);

		sprites[1] = std::make_unique<NYSprite>(renderDevice);
		sprites[1]->translation = glm::vec3(-2.0f, 0.0f, 0.0f);
		sprites[1]->rotation = glm::vec3(0.0f, 0.0f, 0.0f);
		sprites[1]->scale = glm::vec3(2.0f, 2.0f, 1.0f);
		sprites[1]->writeTexture(textures[1]);

		renderingSystem = std::make_unique<NYRenderingSystem>(*renderer, renderDevice, sprites);
	}

	void Game::update() {
		float delta = 0.0f;
		if (NYInput::isKeyPressed(GLFW_KEY_SPACE)) {
			sprites[0]->writeTexture(textures[1]);
		}
		else {
			sprites[0]->writeTexture(textures[0]);
		}

		NYTimer timer;
		renderingSystem->render(sprites, *pipeline);
		timer.endTimer();
		delta = timer.getSeconds();
		glfwPollEvents();
		//printf("%f ms\n", delta * 1000.0);
		
	}
}