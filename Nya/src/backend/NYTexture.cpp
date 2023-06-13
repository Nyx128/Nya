#include "pch.hpp"
#include "NYTexture.hpp"
#define STB_IMAGE_IMPLEMENTATION
#include "stb/stb_image.h"
#include "logging/NYLogger.hpp"
#include "utils/NYTimer.hpp"

namespace Nya {
	NYTexture::NYTexture(NYRenderDevice& _renderDevice, std::string _filepath)
		:renderDevice(_renderDevice), filepath(_filepath) {
		createImage();
		createImageView();
		createSampler();
	}

	NYTexture::~NYTexture(){
		vkDestroySampler(renderDevice.getDevice(), imageSampler, nullptr);
		vkDestroyImageView(renderDevice.getDevice(), imageView ,nullptr);
		vmaDestroyImage(renderDevice.getAllocator(), image, imageAlloc);
	}

	void NYTexture::createImage(){
		stbi_uc* pixels = stbi_load(filepath.c_str(), &width, &height, &channels, STBI_rgb_alpha);

		vk::DeviceSize imageSize = width * height * 4;
		

		if (!pixels) {
			NYLogger::logError("Failed to load image %s", filepath.c_str());
		}

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;

		vk::ImageCreateInfo imageInfo;
		imageInfo.imageType = vk::ImageType::e2D;
		imageInfo.extent.width = static_cast<uint32_t>(width);
		imageInfo.extent.height = static_cast<uint32_t>(height);
		imageInfo.extent.depth = 1;
		imageInfo.mipLevels = 1;
		imageInfo.arrayLayers = 1;
		imageInfo.format = vk::Format::eR8G8B8A8Srgb;
		imageInfo.tiling = vk::ImageTiling::eOptimal;
		imageInfo.initialLayout = vk::ImageLayout::eUndefined;
		imageInfo.usage = vk::ImageUsageFlagBits::eTransferDst | vk::ImageUsageFlagBits::eSampled;
		imageInfo.sharingMode = vk::SharingMode::eExclusive;
		imageInfo.samples = vk::SampleCountFlagBits::e1;

		renderDevice.createImage(image, static_cast<VkImageCreateInfo>(imageInfo), allocInfo, imageAlloc);

		VkBuffer stagingBuffer;
		VmaAllocation stagingAllocation;

		VmaAllocationCreateInfo stagingAllocInfo{};
		stagingAllocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		std::array<uint32_t, 1> queueFamilyIndices = { renderDevice.getGraphicsQueueFamilyIndex() };

		vk::BufferCreateInfo stagingBufferInfo;
		stagingBufferInfo.setQueueFamilyIndices(queueFamilyIndices);
		stagingBufferInfo.usage = vk::BufferUsageFlagBits::eTransferSrc;
		stagingBufferInfo.size = imageSize;
		stagingBufferInfo.sharingMode = vk::SharingMode::eExclusive;

		auto stagingBuffInfo = static_cast<VkBufferCreateInfo>(stagingBufferInfo);

		NYLogger::checkAssert(vmaCreateBuffer(renderDevice.getAllocator(), &stagingBuffInfo, &stagingAllocInfo, &stagingBuffer, &stagingAllocation, nullptr) == VK_SUCCESS, "Failed to create staging buffer for image");

		void* data;
		vmaMapMemory(renderDevice.getAllocator(), stagingAllocation, &data);
		memcpy(data, pixels, imageSize);
		vmaUnmapMemory(renderDevice.getAllocator(), stagingAllocation);
		stbi_image_free(pixels);

		transitionLayout(image, vk::Format::eR8G8B8A8Srgb, VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL);
		copyBufferToImage(stagingBuffer, image, width, height);
		transitionLayout(image, vk::Format::eR8G8B8A8Srgb, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL);
		vmaDestroyBuffer(renderDevice.getAllocator(), stagingBuffer, stagingAllocation);
	}

	void NYTexture::createImageView(){
		VkImageViewCreateInfo viewInfo{};
		viewInfo.sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO;
		viewInfo.image = image;
		viewInfo.viewType = VK_IMAGE_VIEW_TYPE_2D;
		viewInfo.format = VK_FORMAT_R8G8B8A8_SRGB;
		viewInfo.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		viewInfo.subresourceRange.baseMipLevel = 0;
		viewInfo.subresourceRange.levelCount = 1;
		viewInfo.subresourceRange.baseArrayLayer = 0;
		viewInfo.subresourceRange.layerCount = 1;

		NYLogger::checkAssert(vkCreateImageView(renderDevice.getDevice(), &viewInfo, nullptr, &imageView) == VK_SUCCESS,
			"Failed to create image view");
	}

	void NYTexture::createSampler(){
		VkSamplerCreateInfo samplerInfo{};
		samplerInfo.sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO;
		samplerInfo.anisotropyEnable = VK_TRUE;
		vk::PhysicalDeviceProperties phyDeviceProps = renderDevice.getPhysicalDevice().getProperties();
		samplerInfo.maxAnisotropy = phyDeviceProps.limits.maxSamplerAnisotropy;
		samplerInfo.borderColor = VK_BORDER_COLOR_INT_OPAQUE_BLACK;
		samplerInfo.unnormalizedCoordinates = VK_FALSE;
		samplerInfo.compareEnable = VK_FALSE;
		samplerInfo.compareOp = VK_COMPARE_OP_ALWAYS;
		samplerInfo.mipmapMode = VK_SAMPLER_MIPMAP_MODE_LINEAR;
		samplerInfo.mipLodBias = 0.0f;
		samplerInfo.minLod = 0.0f;
		samplerInfo.maxLod = 0.0f;

		NYLogger::checkAssert(vkCreateSampler(renderDevice.getDevice(), &samplerInfo, nullptr, &imageSampler) == VK_SUCCESS,
			"Failed to create image sampler");
	}

	void NYTexture::transitionLayout(VkImage& image, vk::Format format, VkImageLayout oldLayout, VkImageLayout newLayout){
		vk::CommandBuffer commandBuffer = renderDevice.beginSingleTimeCommandBuffers();

		VkImageMemoryBarrier barrier{};
		barrier.sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER;
		barrier.oldLayout = oldLayout;
		barrier.newLayout = newLayout;
		barrier.image = image;
		barrier.srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED;
		barrier.subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		barrier.subresourceRange.baseMipLevel = 0;
		barrier.subresourceRange.levelCount = 1;
		barrier.subresourceRange.baseArrayLayer = 0;
		barrier.subresourceRange.layerCount = 1;

		VkPipelineStageFlags sourceStage;
		VkPipelineStageFlags destinationStage;

		if (oldLayout == VK_IMAGE_LAYOUT_UNDEFINED && newLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_NONE;
			barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;

			sourceStage = VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT;
			destinationStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
		}

		if (oldLayout == VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL && newLayout == VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL) {
			barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
			barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;

			sourceStage = VK_PIPELINE_STAGE_TRANSFER_BIT;
			destinationStage = VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT;
		}
		vkCmdPipelineBarrier(commandBuffer, sourceStage, destinationStage, VK_DEPENDENCY_BY_REGION_BIT, 0, nullptr, 0, nullptr, 1, &barrier);

		renderDevice.endSingleTimeCommandBuffers(commandBuffer);
	}

	void NYTexture::copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height){

		vk::CommandBuffer commandBuffer = renderDevice.beginSingleTimeCommandBuffers();

		VkBufferImageCopy region{};
		region.bufferOffset = 0;
		region.bufferRowLength = 0;
		region.bufferImageHeight = 0;

		region.imageSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT;
		region.imageSubresource.mipLevel = 0;
		region.imageSubresource.baseArrayLayer = 0;
		region.imageSubresource.layerCount = 1;
		
		region.imageOffset = { 0, 0, 0 };
		region.imageExtent = {
			width,
			height,
			1
		};

		vkCmdCopyBufferToImage(commandBuffer, buffer, image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &region);

		renderDevice.endSingleTimeCommandBuffers(commandBuffer);
	}
}