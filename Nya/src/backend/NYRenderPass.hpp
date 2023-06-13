#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"
#include "logging/NYLogger.hpp"

namespace Nya {
	class NYRenderPass {
	public:
		NYRenderPass(NYRenderDevice& _renderDevice);
		~NYRenderPass();

		NYRenderPass(NYRenderPass const&) = delete;
		NYRenderPass& operator=(NYRenderPass const&) = delete;

		vk::RenderPass& getRenderpass() {
			NYLogger::checkAssert(built, "Can't acquire renderpass before it's built");
			return renderPass; }

		void addAttachment(vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout);
		void addSubpass(std::vector<vk::AttachmentReference>& colorAttachments, std::vector<vk::AttachmentReference>& inputAttachments, vk::AttachmentReference* depthAttachment);
		void addDependency(vk::SubpassDependency dependency);

		void build();
		void begin(vk::CommandBuffer& commandBuffer, vk::Framebuffer frameBuffer, vk::Extent2D extent, glm::vec4 clearColor);
		void end(vk::CommandBuffer& commandBuffer);

	private:
		NYRenderDevice& renderDevice;

		//all the attachments used by the renderpass
		std::vector<vk::AttachmentDescription> attachments;

		std::vector<vk::SubpassDescription> subpasses;
		
		std::vector<vk::SubpassDependency> dependencies;

		vk::RenderPass renderPass;

		bool built = false;
		bool start = false;
	};
}
