#include "pch.hpp"
#include "NYFramebuffer.hpp"

namespace Nya {
	NYFramebuffer::NYFramebuffer(NYRenderDevice& _renderDevice, NYSwapchain& _swapchain, NYRenderPass& _renderPass)
		:renderDevice(_renderDevice), renderPass(_renderPass), swapchain(_swapchain) {

	}

	NYFramebuffer::~NYFramebuffer(){

	}

	void NYFramebuffer::createFramebufferAttachment(NYFramebufferAttachment& framebufferAttachment, vk::Format format, vk::ImageUsageFlags usageFlags, int width, int height){
		vk::ImageCreateInfo imageInfo;
		imageInfo.format = format;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = width;
		imageInfo.extent.height = height;
		imageInfo.extent.depth = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.mipLevels = 1;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.samples = vk::SampleCountFlagBits::e1;

		imageInfo.usage = usageFlags;

		VmaAllocationCreateInfo allocInfo;
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		
		renderDevice.createImage(framebufferAttachment.image, static_cast<VkImageCreateInfo>(imageInfo), allocInfo, framebufferAttachment.imageAlloc);

		vk::ImageViewCreateInfo viewInfo;
		viewInfo.format = format;
		viewInfo.image = framebufferAttachment.image;
		viewInfo.subresourceRange.aspectMask = vk::ImageAspectFlagBits::eColor;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.layerCount = 1;

		framebufferAttachment.imageView = renderDevice.getDevice().createImageView(viewInfo);
	}
}