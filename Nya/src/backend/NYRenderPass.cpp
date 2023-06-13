#include "pch.hpp"
#include "NYRenderPass.hpp"
#include "logging/NYLogger.hpp"

namespace Nya {
	NYRenderPass::NYRenderPass(NYRenderDevice& _renderDevice) :renderDevice(_renderDevice) {

	}

	NYRenderPass::~NYRenderPass() {
		NYLogger::checkAssert(built, "build() must be called before NYRenderPass' destructor");
		renderDevice.getDevice().destroyRenderPass(renderPass);
	}

	void NYRenderPass::addAttachment(vk::Format format, vk::AttachmentLoadOp loadOp, vk::AttachmentStoreOp storeOp, vk::ImageLayout initialLayout, vk::ImageLayout finalLayout){
		NYLogger::checkAssert(!built, "NYRenderPass can't be modified after it's built");

		vk::AttachmentDescription attachment;
		attachment.samples = vk::SampleCountFlagBits::e1;
		attachment.format = format;
		attachment.loadOp = loadOp;
		attachment.storeOp = storeOp;
		//ignore these for now
		attachment.stencilLoadOp = vk::AttachmentLoadOp::eDontCare;
		attachment.stencilStoreOp = vk::AttachmentStoreOp::eDontCare;
		attachment.initialLayout = initialLayout;
		attachment.finalLayout = finalLayout;

		attachments.push_back(attachment);
	}

	void NYRenderPass::addSubpass(std::vector<vk::AttachmentReference>& colorAttachments, std::vector<vk::AttachmentReference>& inputAttachments, vk::AttachmentReference* depthAttachment){
		NYLogger::checkAssert(!built, "NYRenderPass can't be modified after it's built");

		vk::SubpassDescription subpass;
		if (colorAttachments.size() != 0) {
			subpass.setColorAttachments(colorAttachments);
		}
		if (inputAttachments.size() != 0) { 
			subpass.setInputAttachments(inputAttachments); 
		}
		if (depthAttachment != nullptr) {
			subpass.setPDepthStencilAttachment(depthAttachment);
		}
		subpasses.push_back(subpass);
	}

	void NYRenderPass::addDependency(vk::SubpassDependency dependency) {
		NYLogger::checkAssert(!built, "NYRenderPass can't be modified after it's built");

		dependencies.push_back(dependency);
	}

	void NYRenderPass::build() {
		vk::RenderPassCreateInfo passInfo;
		passInfo.setAttachments(attachments);
		passInfo.setSubpasses(subpasses);
		passInfo.setDependencies(dependencies);


		renderPass = renderDevice.getDevice().createRenderPass(passInfo);
		built = true;
	}

	void NYRenderPass::begin(vk::CommandBuffer& commandBuffer, vk::Framebuffer frameBuffer, vk::Extent2D extent, glm::vec4 clearColor){
		NYLogger::checkAssert(built, "Can't begin render pass without building it");
		vk::RenderPassBeginInfo renderPassBeginInfo;
		renderPassBeginInfo.setFramebuffer(frameBuffer);
		renderPassBeginInfo.setClearValueCount(1);

		vk::ClearValue clearValue;
		std::array<float, 4> _color = { clearColor.x, clearColor.y, clearColor.z, clearColor.w };
		vk::ClearColorValue clearColorVal(_color);
		clearValue.color = clearColorVal;
		std::array<vk::ClearValue, 1> clearValues = { clearValue };

		renderPassBeginInfo.setClearValues(clearValues);
		renderPassBeginInfo.setRenderPass(renderPass);

		vk::Rect2D renderArea;
		renderArea.setOffset(vk::Offset2D(0.0f, 0.0f));
		renderArea.setExtent(extent);

		renderPassBeginInfo.setRenderArea(renderArea);

		commandBuffer.beginRenderPass(renderPassBeginInfo, vk::SubpassContents::eInline);
		start = true;
	}

	void NYRenderPass::end(vk::CommandBuffer& commandBuffer) {
		NYLogger::checkAssert(start, "Can't end a render pass without beginning it");
		commandBuffer.endRenderPass();
	}
}

