#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"

/*
This is the swapchain abstraction class, it will encapsulate the following things
-swapchain framebuffers
-swapchain images
-swapchain image views
-image formats, presentation modes
*/

namespace Nya {
	class NYSwapchain {
	public:
		NYSwapchain(NYRenderDevice& _renderDevice);
		~NYSwapchain();

		NYSwapchain(NYSwapchain const&) = delete;
		NYSwapchain& operator=(NYSwapchain const&) = delete;

		vk::SwapchainKHR& getSwapchain() { return swapchain; }

		vk::Extent2D getSwapchainExtent() { return swapchainExtent; }
		vk::Format getSwapchainFormat() { return surfFormat.format; }

		vk::Framebuffer getFrameBuffer(uint32_t index) { return frameBuffers[index]; }

		size_t numSwapchainImages() { return swapchainImages.size(); }
		vk::SurfaceCapabilitiesKHR getSwapchainCapabilities() { return surfCapabilities; }
		vk::ImageView* getImageView_Ptr(uint32_t imageIndex) { return &imageViews[imageIndex]; }

		//create the pipeline first and then use its renderPass
		void createFrameBuffers(vk::RenderPass& renderPass);
	private:
		void getSwapchainInfo();
		void selectParams();
		void createSwapchain();
		void createImageViews();

		NYRenderDevice& renderDevice;//as mentioned before we cant copy this take it as reference

		vk::SwapchainKHR swapchain;

		vk::SurfaceCapabilitiesKHR surfCapabilities;
		std::vector<vk::SurfaceFormatKHR> surfFormats;
		std::vector<vk::PresentModeKHR> presentModes;

		//swapchain resources
		std::vector<vk::ImageView> imageViews;
		std::vector<vk::Image> swapchainImages;

		std::vector<vk::Framebuffer> frameBuffers;

		//initialization parameters
		vk::PresentModeKHR presentMode;
		vk::SurfaceFormatKHR surfFormat;
		vk::Extent2D swapchainExtent;

		bool createdFrameBuffers = false;

	};
}
