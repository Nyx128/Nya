#pragma once
#include "pch.hpp"
#include "NYRenderPass.hpp"
#include "backend/NYRenderDevice.hpp"
#include "NYSwapchain.hpp"

namespace Nya {
	class NYFramebuffer {
	public:
		NYFramebuffer(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain,  NYRenderPass& _renderPass);
		~NYFramebuffer();

		NYFramebuffer(NYFramebuffer const&) = delete;
		NYFramebuffer& operator=(NYFramebuffer const&) = delete;

		struct NYFramebufferAttachment {
			VkImage image;
			VkImageView imageView;
			VmaAllocation imageAlloc;
		};

		void createFramebufferAttachment(NYFramebufferAttachment& framebufferAttachment, vk::Format format, vk::ImageUsageFlags usageFlags, int width, int height);
	private:
		NYRenderPass& renderPass;
		NYRenderDevice& renderDevice;
		NYSwapchain& swapchain;

		std::vector<NYFramebufferAttachment> attachments;
		vk::Framebuffer framebuffer;
	};
}