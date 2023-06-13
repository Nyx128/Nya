#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"

namespace Nya {
	class NYTexture {
	public:
		NYTexture(NYRenderDevice& _renderDevice, std::string _filepath);
		~NYTexture();

		NYTexture(NYTexture const&) = delete;
		NYTexture& operator=(NYTexture const&) = delete;

		VkImageView& getImageView() { return imageView; }
		VkSampler& getSampler() { return imageSampler; }
	private:

		void createImage();
		void createImageView();
		void createSampler();

		void transitionLayout(VkImage& image, vk::Format format, VkImageLayout oldLayout, VkImageLayout newLayout);
		void copyBufferToImage(VkBuffer& buffer, VkImage& image, uint32_t width, uint32_t height);


		NYRenderDevice& renderDevice;

		VkImage image;
		VkImageView imageView;
		VmaAllocation imageAlloc;
		VkSampler imageSampler;

		std::string filepath;

		int width, height, channels;
	};
}
