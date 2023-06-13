#include "pch.hpp"
#include "NYSwapchain.hpp"
#include "logging/NYLogger.hpp"

namespace Nya {
	NYSwapchain::NYSwapchain(NYRenderDevice& _renderDevice):renderDevice(_renderDevice){
		getSwapchainInfo();
		selectParams();
		createSwapchain();
		createImageViews();
		NYLogger::logTrace("NYSwapchain created");
	}

	NYSwapchain::~NYSwapchain(){

		for (auto frameBuffer : frameBuffers) {
			renderDevice.getDevice().destroyFramebuffer(frameBuffer);
		}

		for (auto& imgView : imageViews) {
			renderDevice.getDevice().destroyImageView(imgView);
		}
		renderDevice.getDevice().destroySwapchainKHR(swapchain);
		NYLogger::logTrace("NYSwapchain destroyed");
	}

	void NYSwapchain::getSwapchainInfo(){
		auto surf = static_cast<vk::SurfaceKHR>(renderDevice.getSurface());
		surfCapabilities = renderDevice.getPhysicalDevice().getSurfaceCapabilitiesKHR(surf);
		surfFormats = renderDevice.getPhysicalDevice().getSurfaceFormatsKHR(surf);
		presentModes = renderDevice.getPhysicalDevice().getSurfacePresentModesKHR(surf);
	}

	void NYSwapchain::selectParams(){
		//lets first select which present mode to use
		for (const auto& mode : presentModes) {
			//but this is better
			if (mode == vk::PresentModeKHR::eMailbox) {
				presentMode = vk::PresentModeKHR::eMailbox;
			}
			//since this is always supported
			else if (mode == vk::PresentModeKHR::eFifo) {
				presentMode = vk::PresentModeKHR::eFifo;
				break;
			}
		}
		//now for the swapchain extent
		if (surfCapabilities.currentExtent.width != UINT32_MAX) {
			swapchainExtent = surfCapabilities.currentExtent;
		}
		else {
			int width, height;
			glfwGetFramebufferSize(renderDevice.getWindow().getHandlePointer(), &width, &height);

			swapchainExtent = vk::Extent2D((uint32_t)width, (uint32_t)height);

			swapchainExtent.width = std::clamp(swapchainExtent.width,
											  surfCapabilities.minImageExtent.width,
											  surfCapabilities.maxImageExtent.width);

			swapchainExtent.height = std::clamp(swapchainExtent.height,
											   surfCapabilities.minImageExtent.height,
					                           surfCapabilities.maxImageExtent.height);
		}

		//now the surface format
		
		for (const auto& format : surfFormats) {
			if (format.format == vk::Format::eB8G8R8A8Unorm && format.colorSpace == vk::ColorSpaceKHR::eSrgbNonlinear) {
				surfFormat = format;
			}
		}
		//if it doesnt work select the first one
		surfFormat = surfFormats[0];
	}

	void NYSwapchain::createSwapchain(){

		uint32_t imageCount = surfCapabilities.minImageCount + 1;
		if (surfCapabilities.maxImageCount > 0 && imageCount > surfCapabilities.maxImageCount) {
			imageCount = surfCapabilities.minImageCount;
		}

		std::array<uint32_t, 2> queueFamilyIndices = { renderDevice.getGraphicsQueueFamilyIndex(), renderDevice.getPresentQueueFamilyIndex() };

		vk::SwapchainCreateInfoKHR swapchainInfo;

		if (queueFamilyIndices[0] == queueFamilyIndices[1]) {
			swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(),
				renderDevice.getSurface(),//surface
				imageCount,//minImageCount
				surfFormat.format,//imageFormat
				surfFormat.colorSpace,//colorspace
				swapchainExtent,//extent
				1,//imageArrayLayers
				vk::ImageUsageFlagBits::eColorAttachment,//imageUsage
				vk::SharingMode::eExclusive,//sharingMode
				0,//queueFamilyIndexCount
				nullptr,//pQueueFamilyIndices
				surfCapabilities.currentTransform,//preTransform
				vk::CompositeAlphaFlagBitsKHR::eOpaque,//compositeAlpha
				presentMode,//present mode
				false,//clipped
				nullptr//oldSwapchain
			);
		}
		else {
			swapchainInfo = vk::SwapchainCreateInfoKHR(vk::SwapchainCreateFlagsKHR(),//flags
				renderDevice.getSurface(),//surface
				imageCount,//minImageCount
				surfFormat.format,//imageFormat
				surfFormat.colorSpace,//colorspace
				swapchainExtent,//extent
				1,//imageArrayLayers
				vk::ImageUsageFlagBits::eColorAttachment,//imageUsage
				vk::SharingMode::eConcurrent,//sharingMode
				2,//queueFamilyIndexCount
				queueFamilyIndices.data(),//pQueueFamilyIndices
				surfCapabilities.currentTransform,//preTransform
				vk::CompositeAlphaFlagBitsKHR::eOpaque,//compositeAlpha
				presentMode,//present mode
				false,//clipped
				nullptr//oldSwapchain
			);
		}

		swapchain = renderDevice.getDevice().createSwapchainKHR(swapchainInfo);

		//create the swapchain swapchainImages
		swapchainImages = renderDevice.getDevice().getSwapchainImagesKHR(swapchain);
	}

	void NYSwapchain::createImageViews(){
		imageViews.resize(swapchainImages.size());

		for (int i = 0; i < swapchainImages.size(); i++) {
			
			vk::ImageViewCreateInfo imgViewCreateInfo(vk::ImageViewCreateFlags(),
													  swapchainImages[i],
				                                      vk::ImageViewType::e2D,
				                                      surfFormat.format,
				                                      vk::ComponentMapping{ vk::ComponentSwizzle::eIdentity,
																			vk::ComponentSwizzle::eIdentity,
																			vk::ComponentSwizzle::eIdentity, 
																			vk::ComponentSwizzle::eIdentity},
				                                      vk::ImageSubresourceRange(vk::ImageAspectFlagBits::eColor, 0, 1, 0, 1));

			imageViews[i] = renderDevice.getDevice().createImageView(imgViewCreateInfo);
		}
	}

	void NYSwapchain::createFrameBuffers(vk::RenderPass& renderPass){
		frameBuffers.resize(swapchainImages.size());
		for (int i = 0; i < swapchainImages.size(); i++) {
			std::array<vk::ImageView, 1> attachments;
			//in the default render pass we only had one color attachment, so for this frame buffer we will only have 1 attachment as well
			attachments[0] = imageViews[i];

			vk::FramebufferCreateInfo createInfo = vk::FramebufferCreateInfo(vk::FramebufferCreateFlags(),
																			 renderPass,
																		     attachments,
																			 swapchainExtent.width,
																			 swapchainExtent.height,
																			 1);

			frameBuffers[i] = renderDevice.getDevice().createFramebuffer(createInfo);
		}

		createdFrameBuffers = true;
	}
}