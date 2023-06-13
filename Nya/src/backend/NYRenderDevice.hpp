#pragma once
#include "pch.hpp"
#include "vk_mem_alloc.h"
#include "NYWindow.hpp"

/*
This class is supposed to the encapsulate the functionality of the following things
- Physical device
- VkDevice
- Graphics Queue, Presentation Queue
- Memory operations, allocations, etc,
-Command buffers
- Common utility
*/

namespace Nya {
	class NYRenderDevice {
	public:
		NYRenderDevice(NYRenderDevice const&) = delete;
		NYRenderDevice& operator=(NYRenderDevice const&) = delete;

		struct NYRenderDeviceCreateInfo {
			const char* appName;
			uint32_t appVersion;
		};

		NYRenderDevice(NYRenderDeviceCreateInfo& _createInfo, NYWindow& _window);
		/*taking a reference as i only need it to be valid during creation and wouldn't mind
		it being invalid after creation when it runs out of scope*/
		~NYRenderDevice();

		//getters
		vk::Device& getDevice() { return device; }
		vk::Instance& getInstance(){return instance; }
		VmaAllocator getAllocator() { return allocator; }
		VkSurfaceKHR& getSurface() { return surface; }
		vk::PhysicalDevice& getPhysicalDevice() { return physicalDevice; }
		NYWindow& getWindow() { return window; }
		uint32_t getPresentQueueFamilyIndex() { return presentQueueFamilyIndex.value(); }
		uint32_t getGraphicsQueueFamilyIndex() { return graphicsQueueFamilyIndex.value(); }
		vk::Queue getGraphicsQueue() { return graphicsQueue; }
		vk::Queue getPresentQueue() { return presentQueue; }

		//utilities
		vk::CommandBuffer beginSingleTimeCommandBuffers();
		void endSingleTimeCommandBuffers(vk::CommandBuffer commandBuffer);
		void createImage(VkImage& Image, const VkImageCreateInfo& ImageInfo, const VmaAllocationCreateInfo& AllocInfo, VmaAllocation& Allocation);
	private:

		//creation functions
		void createInstance(const char* appName, uint32_t appVersion);
#ifdef NY_DEBUG
		void createDebugMessenger();
#endif
		void createDevice();
		void createVmaAllocator();
		void createCommandPool();

		//check functions
		bool checkLayers(std::vector<const char*> const& layers);
		bool checkExtensionSupport(std::vector<const char*> extensions);
		
#ifdef NY_DEBUG
		VkDebugUtilsMessengerEXT debugUtilsMessenger;
#endif
		//private constant variables
		const char* engineName = "Nya";
		const uint32_t enginerVer = VK_MAKE_VERSION(1, 0, 0);

		//window class referenc
		NYWindow& window;

		//private vulkan handles
		vk::Instance instance;
		vk::PhysicalDevice physicalDevice;
		vk::Device device;
		VkSurfaceKHR surface;
		vk::Queue graphicsQueue;
		vk::Queue presentQueue;
		vk::CommandPool commandPool;

		//vulkan memory allocator
		VmaAllocator allocator;

		//necessary variables
		std::optional<uint32_t> graphicsQueueFamilyIndex = 0;
		std::optional<uint32_t> presentQueueFamilyIndex = 0;
	};
}
