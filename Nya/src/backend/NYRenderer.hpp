#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"
#include "NYSwapchain.hpp"
#include "NYPipeline.hpp"
#include "NYTexture.hpp"
#include "NYDescriptorSetLayout.hpp"
#include "NYShader.hpp"
#include "game/NYSprite.hpp"
#include "GUI/NYGUIDevice.hpp"

namespace Nya {
	class NYRenderer {
	public:

		NYRenderer(NYRenderer const&) = delete;
		NYRenderer& operator=(NYRenderer const&) = delete;

		NYRenderer(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain);
		~NYRenderer();

		uint32_t getFrameIndex() { return  currentFrame; }
		
		void beginRenderPass(glm::vec4 clearColor, NYPipeline& pipeline);
		void bindPipeline(NYPipeline& pipeline);
		void pushConstants(NYPipeline& pipeline, void* pushData, uint32_t size);
		void draw(VkBuffer& vertexBuffer, VkBuffer& indexBuffer,  NYPipeline& pipeline, uint32_t indexCount, vk::DescriptorSet& set);
		void endRenderPass(NYPipeline& pipeline);
	private:
		uint32_t currentFrame = 0;
		uint32_t imageIndex = 0;

		void createOffscreenFramebufferResources();
		void createSyncObjects();
		void guiCalls();

		void acquireImageIndex();
		void submitCommands();

		NYRenderDevice& renderDevice;
		NYSwapchain& swapchain;

		NYGUIDevice guiDevice{ renderDevice, swapchain };

		//getting these handles beforehand for that lil bit of extra performance
		vk::Device& deviceHandle;
		const vk::Queue& graphicsQueue = renderDevice.getGraphicsQueue();
		const vk::Queue& presentQueue = renderDevice.getPresentQueue();

		//renderer resources
		vk::CommandPool commandPool;
		std::vector<vk::CommandBuffer> commandBuffers;
		std::vector<vk::Semaphore> imageAvailableSemaphores;
		std::vector<vk::Semaphore> renderFinishedSemaphores;
		std::vector<vk::Fence> inFlightFences;
		
	};
}
