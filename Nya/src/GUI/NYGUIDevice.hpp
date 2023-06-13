#pragma once
#include "pch.hpp"
#include "backend/NYRenderDevice.hpp"
#include "backend/NYSwapchain.hpp"
#include "backend/NYRenderPass.hpp"
#include "backend/NYPipeline.hpp"

namespace Nya {
	class NYGUIDevice {
	public:
		NYGUIDevice(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain);
		~NYGUIDevice();

		NYGUIDevice(NYGUIDevice const&) = delete;
		NYGUIDevice& operator=(NYGUIDevice const&) = delete;

		void recordCommands(vk::CommandBuffer& commandBuffer, NYPipeline& pipeline, uint32_t imageIndex);
	private:
		NYRenderDevice& renderDevice;
		NYSwapchain& swapchain;

		NYRenderPass guiPass{renderDevice};

		vk::DescriptorPool pool;
		std::vector<vk::Framebuffer> framebuffers;

		void createFramebuffers();
	};
}
