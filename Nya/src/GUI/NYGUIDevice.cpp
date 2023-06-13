#include "pch.hpp"
#include "NYGUIDevice.hpp"

namespace Nya {
	NYGUIDevice::NYGUIDevice(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain):renderDevice(_renderDevice), swapchain(_swapchain) {
		IMGUI_CHECKVERSION();
		ImGui::CreateContext();
		ImGuiIO& io = ImGui::GetIO(); (void)io;
		io.ConfigFlags |= ImGuiConfigFlags_DockingEnable;

		ImGui::StyleColorsDracula();

		std::vector<vk::DescriptorPoolSize> poolSizes = {
			{ vk::DescriptorType::eSampler, 1000 },
			{ vk::DescriptorType::eCombinedImageSampler, 1000 },
			{ vk::DescriptorType::eSampledImage, 1000 },
			{ vk::DescriptorType::eStorageImage, 1000 },
			{ vk::DescriptorType::eUniformTexelBuffer, 1000 },
			{ vk::DescriptorType::eStorageTexelBuffer, 1000 },
			{ vk::DescriptorType::eUniformBuffer, 1000 },
			{ vk::DescriptorType::eStorageBuffer, 1000 },
			{ vk::DescriptorType::eUniformBufferDynamic, 1000 },
			{ vk::DescriptorType::eStorageBufferDynamic, 1000 },
			{ vk::DescriptorType::eInputAttachment, 1000 }
		};

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eFreeDescriptorSet;
		poolInfo.maxSets = 1000 * poolSizes.size();
		poolInfo.poolSizeCount = static_cast<uint32_t>(poolSizes.size());
		poolInfo.pPoolSizes = poolSizes.data();

		 pool = renderDevice.getDevice().createDescriptorPool(poolInfo);
		ImGui_ImplGlfw_InitForVulkan(renderDevice.getWindow().getHandlePointer(), true);

		ImGui_ImplVulkan_InitInfo init_info = {};
		init_info.Instance = static_cast<VkInstance>(renderDevice.getInstance());
		init_info.PhysicalDevice = static_cast<VkPhysicalDevice>(renderDevice.getPhysicalDevice());
		init_info.Device = static_cast<VkDevice>(renderDevice.getDevice());
		init_info.QueueFamily = renderDevice.getGraphicsQueueFamilyIndex();
		init_info.Queue = static_cast<VkQueue>(renderDevice.getGraphicsQueue());
		init_info.PipelineCache = VK_NULL_HANDLE;
		init_info.DescriptorPool = pool;
		init_info.Allocator = nullptr;
		init_info.MinImageCount = swapchain.getSwapchainCapabilities().minImageCount;
		init_info.ImageCount = swapchain.numSwapchainImages();
		init_info.CheckVkResultFn = nullptr;

		guiPass.addAttachment(swapchain.getSwapchainFormat(), vk::AttachmentLoadOp::eLoad, vk::AttachmentStoreOp::eStore, vk::ImageLayout::eColorAttachmentOptimal, vk::ImageLayout::ePresentSrcKHR);
		
		vk::AttachmentReference colorAttachmentRef = vk::AttachmentReference(0, vk::ImageLayout::eColorAttachmentOptimal);
		std::vector<vk::AttachmentReference> colorAttachmentRefs;
		colorAttachmentRefs.push_back(colorAttachmentRef);

		std::vector<vk::AttachmentReference> inputAttachmentRefs;

		guiPass.addSubpass(colorAttachmentRefs, inputAttachmentRefs, nullptr);

		vk::SubpassDependency dependency;
		dependency.setSrcSubpass(VK_SUBPASS_EXTERNAL);
		dependency.setDstSubpass(0);
		dependency.setSrcAccessMask(vk::AccessFlagBits::eNone);
		dependency.setDstAccessMask(vk::AccessFlagBits::eColorAttachmentWrite);
		dependency.setSrcStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);
		dependency.setDstStageMask(vk::PipelineStageFlagBits::eColorAttachmentOutput);

		guiPass.build();
		ImGui_ImplVulkan_Init(&init_info, static_cast<VkRenderPass>(guiPass.getRenderpass()));

		vk::CommandBuffer commandBuffer = renderDevice.beginSingleTimeCommandBuffers();
		ImGui_ImplVulkan_CreateFontsTexture(static_cast<VkCommandBuffer>(commandBuffer));
		renderDevice.endSingleTimeCommandBuffers(commandBuffer);

		createFramebuffers();
	}

	NYGUIDevice::~NYGUIDevice(){
		ImGui_ImplVulkan_Shutdown();
		ImGui_ImplGlfw_Shutdown();
		renderDevice.getDevice().destroyDescriptorPool(pool);
		for (int i = 0; i < framebuffers.size(); i++) {
			renderDevice.getDevice().destroyFramebuffer(framebuffers[i]);
		}
		ImGui::DestroyContext();
	}

	void NYGUIDevice::recordCommands(vk::CommandBuffer& commandBuffer, NYPipeline& pipeline, uint32_t imageIndex){
		guiPass.begin(commandBuffer, framebuffers[imageIndex], swapchain.getSwapchainExtent(), glm::vec4(1.0));
		commandBuffer.bindPipeline(vk::PipelineBindPoint::eGraphics, pipeline.getPipeline());
		auto draw_data = ImGui::GetDrawData();
		ImGui_ImplVulkan_RenderDrawData(draw_data, commandBuffer, NULL);
		guiPass.end(commandBuffer);
	}

	void NYGUIDevice::createFramebuffers(){
		framebuffers.resize(swapchain.numSwapchainImages());
		for (int i = 0; i < framebuffers.size(); i++) {
			vk::FramebufferCreateInfo createInfo;
			createInfo.attachmentCount = 1;
			createInfo.pAttachments = swapchain.getImageView_Ptr(i);
			createInfo.layers = 1;
			createInfo.setWidth(swapchain.getSwapchainExtent().width);
			createInfo.setHeight(swapchain.getSwapchainExtent().height);
			createInfo.setRenderPass(guiPass.getRenderpass());

			framebuffers[i] = renderDevice.getDevice().createFramebuffer(createInfo);
		}
	}
}