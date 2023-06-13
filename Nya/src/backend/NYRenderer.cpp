#include "pch.hpp"
#include "NYRenderer.hpp"
#include "systems/NYRenderingSystem.hpp"
#include "logging/NYLogger.hpp"
#include "utils/NYTimer.hpp"
#include "glm/gtc/type_ptr.hpp"
#include "defines.hpp"

namespace Nya {
	NYRenderer::NYRenderer(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain)
		:renderDevice(_renderDevice), swapchain(_swapchain), deviceHandle(_renderDevice.getDevice()) {
		//create a separate command pool for all rendering commands
		//make it resetable to reuse command buffers
		vk::CommandPoolCreateInfo poolInfo(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eResetCommandBuffer),
			renderDevice.getGraphicsQueueFamilyIndex()
		);

		commandPool = renderDevice.getDevice().createCommandPool(poolInfo);

		commandBuffers.resize(MAX_FRAMES_IN_FLIGHT);

		//allocate command buffers
		vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, MAX_FRAMES_IN_FLIGHT);
		commandBuffers = renderDevice.getDevice().allocateCommandBuffers(allocInfo);

		createSyncObjects();
	}

	NYRenderer::~NYRenderer(){

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			deviceHandle.destroySemaphore(imageAvailableSemaphores[i]);
			deviceHandle.destroySemaphore(renderFinishedSemaphores[i]);
			deviceHandle.destroyFence(inFlightFences[i]);
		}
		
		deviceHandle.destroyCommandPool(commandPool);
	}

	void NYRenderer::createOffscreenFramebufferResources()
	{
	}

	void NYRenderer::createSyncObjects(){
		imageAvailableSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		renderFinishedSemaphores.resize(MAX_FRAMES_IN_FLIGHT);
		inFlightFences.resize(MAX_FRAMES_IN_FLIGHT);

		vk::SemaphoreCreateInfo semaphoreInfo;
		vk::FenceCreateInfo fenceInfo;
		fenceInfo.setFlags(vk::FenceCreateFlagBits::eSignaled);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			imageAvailableSemaphores[i] = deviceHandle.createSemaphore(semaphoreInfo);
			renderFinishedSemaphores[i] = deviceHandle.createSemaphore(semaphoreInfo);
			inFlightFences[i] = deviceHandle.createFence(fenceInfo);
		}


	}

	void NYRenderer::guiCalls(){

		/*{
			static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

			// We are using the ImGuiWindowFlags_NoDocking flag to make the parent window not dockable into,
			// because it would be confusing to have two docking targets within each others.
			ImGuiWindowFlags window_flags = ImGuiWindowFlags_NoDocking;

			const ImGuiViewport* viewport = ImGui::GetMainViewport();
			ImGui::SetNextWindowPos(viewport->WorkPos);
			ImGui::SetNextWindowSize(viewport->WorkSize);
			ImGui::SetNextWindowViewport(viewport->ID);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
			ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
			window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
			window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;

			// When using ImGuiDockNodeFlags_PassthruCentralNode, DockSpace() will render our background
			// and handle the pass-thru hole, so we ask Begin() to not render a background.
			if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
				window_flags |= ImGuiWindowFlags_NoBackground;

			// Important: note that we proceed even if Begin() returns false (aka window is collapsed).
			// This is because we want to keep our DockSpace() active. If a DockSpace() is inactive,
			// all active windows docked into it will lose their parent and become undocked.
			// We cannot preserve the docking relationship between an active window and an inactive docking, otherwise
			// any change of dockspace/settings would lead to windows being stuck in limbo and never being visible.
			ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
			ImGui::Begin("DockSpace Demo", nullptr, window_flags);
			ImGui::PopStyleVar();

			ImGui::PopStyleVar(2);

			// Submit the DockSpace
			ImGuiIO& io = ImGui::GetIO();
			if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
			{
				ImGuiID dockspace_id = ImGui::GetID("VulkanAppDockspace");
				ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);
			}

			ImGui::End();
		}*/

		ImGui::Begin("Debug window");
		ImGui::Text("Application average %.3f ms/frame (%.1f FPS)", 1000.0f / ImGui::GetIO().Framerate, ImGui::GetIO().Framerate);
		ImGui::End();
		ImGui::Render();
	}

	void NYRenderer::acquireImageIndex() {
		deviceHandle.waitForFences(inFlightFences[currentFrame], true, UINT64_MAX);
		deviceHandle.resetFences(1, &inFlightFences[currentFrame]);


		vk::Result result = renderDevice.getDevice().acquireNextImageKHR(swapchain.getSwapchain(), UINT64_MAX, imageAvailableSemaphores[currentFrame], VK_NULL_HANDLE, &imageIndex);
		NYLogger::checkAssert(result == vk::Result::eSuccess, "Failed to acquire image from swapchain");
	}

	void NYRenderer::pushConstants(NYPipeline& pipeline ,void* pushData, uint32_t size) {
		commandBuffers[currentFrame].pushConstants(pipeline.getLayout(), vk::ShaderStageFlagBits::eVertex, 0, size, pushData);
	}


	void NYRenderer::beginRenderPass(glm::vec4 color, NYPipeline& pipeline) {
		acquireImageIndex();

		ImGui_ImplVulkan_NewFrame();
		ImGui_ImplGlfw_NewFrame();
		ImGui::NewFrame();

		guiCalls();

		commandBuffers[currentFrame].reset();
		vk::CommandBufferBeginInfo cBeginInfo;
		commandBuffers[currentFrame].begin(cBeginInfo);

		pipeline.getRenderPass().begin(commandBuffers[currentFrame], swapchain.getFrameBuffer(imageIndex), swapchain.getSwapchainExtent(), glm::vec4(0.05f, 0.05f, 0.05f, 1.0f));
	}

	void NYRenderer::bindPipeline(NYPipeline& pipeline) {
		commandBuffers[currentFrame].bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());
	}

	void NYRenderer::draw(VkBuffer& vertexBuffer, VkBuffer& indexBuffer, NYPipeline& pipeline, uint32_t indexCount, vk::DescriptorSet& set) {
		std::array<vk::Buffer, 1> vertexBuffers = { static_cast<vk::Buffer>(vertexBuffer) };
		std::array<vk::DeviceSize, 1> offsets = { 0 };
		commandBuffers[currentFrame].bindVertexBuffers(0, vertexBuffers, offsets);
		commandBuffers[currentFrame].bindIndexBuffer(indexBuffer, 0, vk::IndexType::eUint32);
		commandBuffers[currentFrame].bindDescriptorSets(vk::PipelineBindPoint::eGraphics, pipeline.getLayout(), 0, 1, &set, 0, nullptr);

		commandBuffers[currentFrame].drawIndexed(indexCount, 1, 0, 0, 0);
	}

	void NYRenderer::endRenderPass(NYPipeline& pipeline) {
		pipeline.getRenderPass().end(commandBuffers[currentFrame]);
		guiDevice.recordCommands(commandBuffers[currentFrame], pipeline, imageIndex);
		commandBuffers[currentFrame].end();
		
		submitCommands();

		vk::PresentInfoKHR presentInfo;
		presentInfo.setPSwapchains(&swapchain.getSwapchain());
		presentInfo.setSwapchainCount(1);
		presentInfo.setWaitSemaphoreCount(1);
		presentInfo.setPWaitSemaphores(&renderFinishedSemaphores[currentFrame]);
		presentInfo.setPImageIndices(&imageIndex);

		presentQueue.presentKHR(presentInfo);

		currentFrame = (currentFrame + 1) % MAX_FRAMES_IN_FLIGHT;
	}

	void NYRenderer::submitCommands(){
		vk::SubmitInfo submitInfo;

		//each semaphore in this array will be signaled when the corresponding stage in waitStages with the sameIndex is completed
		std::array<vk::Semaphore, 1> waitSemaphores = { imageAvailableSemaphores[currentFrame]};
		std::array<vk::PipelineStageFlags, 1> waitStages = { vk::PipelineStageFlagBits::eColorAttachmentOutput };
		//signal semaphores are signaled when command buffers have completed execution and the rendering is complete
		std::array<vk::Semaphore, 1> signalSemaphores = { renderFinishedSemaphores[currentFrame]};

		submitInfo.setPWaitDstStageMask(waitStages.data());
		submitInfo.setWaitSemaphores(waitSemaphores);
		submitInfo.setCommandBuffers(commandBuffers[currentFrame]);
		submitInfo.setSignalSemaphores(signalSemaphores);

		graphicsQueue.submit(1, &submitInfo, inFlightFences[currentFrame]);
	}
}