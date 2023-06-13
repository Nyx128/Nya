#include "pch.hpp"
#define VMA_IMPLEMENTATION
#include "vk_mem_alloc.h"
#include "NYRenderDevice.hpp"
#include "../logging/NYLogger.hpp"

#ifdef NY_DEBUG
	PFN_vkCreateDebugUtilsMessengerEXT  pfnVkCreateDebugUtilsMessengerEXT;
	PFN_vkDestroyDebugUtilsMessengerEXT pfnVkDestroyDebugUtilsMessengerEXT;

	VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugUtilsMessengerEXT(VkInstance instance,
		const VkDebugUtilsMessengerCreateInfoEXT* pCreateInfo,
		const VkAllocationCallbacks* pAllocator,
		VkDebugUtilsMessengerEXT* pMessenger) {
		return pfnVkCreateDebugUtilsMessengerEXT(instance, pCreateInfo, pAllocator, pMessenger);
	}

	VKAPI_ATTR void VKAPI_CALL vkDestroyDebugUtilsMessengerEXT(VkInstance instance, VkDebugUtilsMessengerEXT messenger,
		VkAllocationCallbacks const* pAllocator) {
		return pfnVkDestroyDebugUtilsMessengerEXT(instance, messenger, pAllocator);
	}

	VKAPI_ATTR VkBool32 VKAPI_CALL debugCallback(VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
		VkDebugUtilsMessageTypeFlagsEXT messageType, const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
		void* pUserData){
		Nya::NYLogger::logFatal(pCallbackData->pMessage);
		return VK_FALSE;
	}
#endif
namespace Nya {
	NYRenderDevice::NYRenderDevice(NYRenderDeviceCreateInfo& _createInfo, NYWindow& _window):window(_window) {
		createInstance(_createInfo.appName, _createInfo.appVersion);
#ifdef NY_DEBUG
		createDebugMessenger();
#endif
		createDevice();
		createVmaAllocator();
		createCommandPool();
		NYLogger::logTrace("NYRenderDevice created");
	}

	NYRenderDevice::~NYRenderDevice(){
		device.destroyCommandPool(commandPool, nullptr);
		vmaDestroyAllocator(allocator);
		device.destroy();
		vkDestroySurfaceKHR(static_cast<VkInstance>(instance), surface, nullptr);
#ifndef NDEBUG
		instance.destroyDebugUtilsMessengerEXT(debugUtilsMessenger);
#endif
		instance.destroy();
		NYLogger::logTrace("NYRenderDevice destroyed");
	}

	bool NYRenderDevice::checkLayers(std::vector<const char*> const& layers) {
		std::vector<vk::LayerProperties> properties = vk::enumerateInstanceLayerProperties();
		//iterate through all the elements of the vector of layer names
		return std::all_of(layers.begin(),
			layers.end(),
			[&properties](const char* name)//the predicate which will be run of every element
			{	
				//iterate through all the property names and check if our layer name is in the properties vector
				return std::find_if(properties.begin(),
					properties.end(),
					[&name](vk::LayerProperties const& property)
					{ return strcmp(property.layerName, name) == 0; }) != properties.end();
			});
	}

	bool NYRenderDevice::checkExtensionSupport(std::vector<const char*> extensions){
		std::vector<vk::ExtensionProperties> extensionProperties = physicalDevice.enumerateDeviceExtensionProperties();

		return std::all_of(extensions.begin(), extensions.end(),
			[&extensionProperties](const char* name) {
				return std::find_if(extensionProperties.begin(), extensionProperties.end(),
					[&name](vk::ExtensionProperties& property) {
						return strcmp(property.extensionName, name) == 0;
					}) != extensionProperties.end();
			});
	}

	void NYRenderDevice::createInstance(const char* appName, uint32_t appVersion) {
		vk::ApplicationInfo appInfo
		(	appName,
			appVersion,
			engineName,
			enginerVer,
			VK_API_VERSION_1_2
		);
		std::vector<const char*> layers;

		uint32_t glfwExtensionCount = 0;
		const char** glfwExtensions;
		glfwExtensions = glfwGetRequiredInstanceExtensions(&glfwExtensionCount);

		std::vector<const char*> extensionNames(glfwExtensions, glfwExtensions + glfwExtensionCount);

		//if debugging is enabled use the validation layers and extensions
#ifdef NY_DEBUG
		layers.push_back("VK_LAYER_KHRONOS_validation");
		extensionNames.push_back(VK_EXT_DEBUG_UTILS_EXTENSION_NAME);
#endif
		if (!checkLayers(layers)) {
			NYLogger::logFatal("Required layers are not available, termination execution");
		}

		vk::InstanceCreateInfo instanceCreateInfo({},
												  &appInfo,
												  static_cast<uint32_t>(layers.size()),
												  layers.data(),
												  static_cast<uint32_t>(extensionNames.size()),
												  extensionNames.data()
												 );
		instance = vk::createInstance(instanceCreateInfo);
		VkResult err = glfwCreateWindowSurface(static_cast<VkInstance>(instance), window.getHandlePointer(), NULL, &surface);

		NYLogger::checkAssert(err == VK_SUCCESS, "failed to create window surface");
	}
#ifdef NY_DEBUG
	void NYRenderDevice::createDebugMessenger(){
		pfnVkCreateDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkCreateDebugUtilsMessengerEXT>(instance.getProcAddr("vkCreateDebugUtilsMessengerEXT"));
		NYLogger::checkAssert(pfnVkCreateDebugUtilsMessengerEXT,
			"GetInstanceProcAddr: Unable to find pfnVkCreateDebugUtilsMessengerEXT function.");

		pfnVkDestroyDebugUtilsMessengerEXT = reinterpret_cast<PFN_vkDestroyDebugUtilsMessengerEXT>(instance.getProcAddr("vkDestroyDebugUtilsMessengerEXT"));
		NYLogger::checkAssert(pfnVkDestroyDebugUtilsMessengerEXT,
			"GetInstanceProcAddr: Unable to find pfnVkDestroyDebugUtilsMessengerEXT function.");

		vk::DebugUtilsMessageSeverityFlagsEXT severityFlags(vk::DebugUtilsMessageSeverityFlagBitsEXT::eWarning |
			vk::DebugUtilsMessageSeverityFlagBitsEXT::eError);

		vk::DebugUtilsMessageTypeFlagsEXT messageTypeFlags(vk::DebugUtilsMessageTypeFlagBitsEXT::eGeneral | vk::DebugUtilsMessageTypeFlagBitsEXT::ePerformance |
			vk::DebugUtilsMessageTypeFlagBitsEXT::eValidation);


		debugUtilsMessenger =
			instance.createDebugUtilsMessengerEXT(vk::DebugUtilsMessengerCreateInfoEXT({}, severityFlags, messageTypeFlags, &debugCallback));
	}
#endif
	void NYRenderDevice::createDevice(){
		//TODO:pick the best gpu on hand
		//get the first compatible gpu
		physicalDevice = instance.enumeratePhysicalDevices().front();
		vk::PhysicalDeviceFeatures features;
		features.fillModeNonSolid = true;
		features.samplerAnisotropy = true;

		vk::PhysicalDeviceDescriptorIndexingFeatures descFeatures;
		descFeatures.descriptorBindingSampledImageUpdateAfterBind = true;
		descFeatures.descriptorBindingUniformBufferUpdateAfterBind = true;

	
		//print device name and api version from the properties
		vk::PhysicalDeviceProperties gpu_props = physicalDevice.getProperties();
#ifdef NY_DEBUG
		NYLogger::logTrace("Max memory allocation count: %d", gpu_props.limits.maxMemoryAllocationCount);
#endif
		const uint32_t apiVer = gpu_props.apiVersion;
		NYLogger::logInfo("VULKAN API VERSION: %d.%d.%d", VK_VERSION_MAJOR(apiVer), VK_VERSION_MINOR(apiVer), VK_VERSION_MAJOR(apiVer));
		NYLogger::logInfo("GPU:%s", gpu_props.deviceName);

		std::vector<vk::QueueFamilyProperties> queueFamilyProperties = physicalDevice.getQueueFamilyProperties();

		//iterator that checks for graphics queue
		auto graphicsQueueIter = std::find_if(queueFamilyProperties.begin(), queueFamilyProperties.end(),
			[](vk::QueueFamilyProperties const& qfp)
			{ return qfp.queueFlags & vk::QueueFlagBits::eGraphics; });

		//now check for present queue
		for (uint32_t i = 0; i < queueFamilyProperties.size(); i++) {
			if (physicalDevice.getSurfaceSupportKHR(i, surface)) { presentQueueFamilyIndex = i; }
		}

		//get its index
		graphicsQueueFamilyIndex = std::distance(queueFamilyProperties.begin(), graphicsQueueIter);

		NYLogger::checkAssert(graphicsQueueFamilyIndex.has_value(), "couldn't find a graphics queue");

		NYLogger::checkAssert(presentQueueFamilyIndex.has_value(), "couldn't find a present queue");

		// create a Device
		float queuePriority = 0.0f;
		/*there can be 2 cases here :
		1)graphicsQueueFamilyIndex and presentQueueFamilyIndex are the same, in which case, we must only use one and fetch both of them
		  at once.It is like this because the vulkan spec states them to be unique
		
		2)other one is that they aren't the same, in which case we make 2 DeviceQueueCreateInfos and pass them as an array*/

		//check for swapchain support
		std::vector<const char*> deviceExtensions = { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
		NYLogger::checkAssert(checkExtensionSupport(deviceExtensions), "Failed to find support for swapchain");

		//enable the swapchain device extension

		if (graphicsQueueFamilyIndex != presentQueueFamilyIndex) {
			vk::DeviceQueueCreateInfo graphicsQueueCreateInfo(vk::DeviceQueueCreateFlags(),
				graphicsQueueFamilyIndex.value(),
				1, &queuePriority);

			vk::DeviceQueueCreateInfo presentQueueCreateInfo(vk::DeviceQueueCreateFlags(),
				presentQueueFamilyIndex.value(),
				1, &queuePriority);

			std::array<vk::DeviceQueueCreateInfo, 2> queueInfos = { graphicsQueueCreateInfo, presentQueueCreateInfo };

			vk::DeviceCreateInfo createInfo;
			createInfo.pNext = &descFeatures;
			createInfo.setPEnabledExtensionNames(deviceExtensions);
			createInfo.pEnabledFeatures = &features;
			createInfo.setQueueCreateInfos(queueInfos);

			device = physicalDevice.createDevice(vk::DeviceCreateInfo(vk::DeviceCreateFlags(), queueInfos));
		}
		else {
			vk::DeviceQueueCreateInfo graphicsQueueCreateInfo(vk::DeviceQueueCreateFlags(),
				graphicsQueueFamilyIndex.value(),
				1, &queuePriority);

			vk::DeviceCreateInfo createInfo;
			createInfo.pNext = &descFeatures;
			createInfo.setPEnabledExtensionNames(deviceExtensions);
			createInfo.pEnabledFeatures = &features;
			createInfo.pQueueCreateInfos = &graphicsQueueCreateInfo;
			createInfo.setQueueCreateInfoCount(1);

			device = physicalDevice.createDevice(createInfo);
		}
		graphicsQueue = device.getQueue(graphicsQueueFamilyIndex.value(), 0);
		presentQueue = device.getQueue(presentQueueFamilyIndex.value(), 0);
	}

	void NYRenderDevice::createVmaAllocator(){
		VmaAllocatorCreateInfo createInfo{};
		createInfo.device = static_cast<VkDevice>(device);
		createInfo.physicalDevice = static_cast<VkPhysicalDevice>(physicalDevice);
		createInfo.instance = static_cast<VkInstance>(instance);
		createInfo.vulkanApiVersion = VK_API_VERSION_1_2;

		auto result = vmaCreateAllocator(&createInfo, &allocator);
		NYLogger::checkAssert(result == VK_SUCCESS, "Failed to create vulkan memory allocator");
	}

	void NYRenderDevice::createCommandPool(){
		vk::CommandPoolCreateInfo createInfo(
			vk::CommandPoolCreateFlags(vk::CommandPoolCreateFlagBits::eTransient),
			graphicsQueueFamilyIndex.value()
		);

		commandPool = device.createCommandPool(createInfo, nullptr);
	}

	vk::CommandBuffer NYRenderDevice::beginSingleTimeCommandBuffers(){
		//utility function used to create single use command buffers 

		vk::CommandBufferBeginInfo beginInfo;
		beginInfo.flags = vk::CommandBufferUsageFlagBits::eOneTimeSubmit;

		vk::CommandBufferAllocateInfo allocInfo(commandPool, vk::CommandBufferLevel::ePrimary, 1);
		auto commandBuffers = device.allocateCommandBuffers(allocInfo);

		commandBuffers[0].begin(beginInfo);
		return commandBuffers[0];
	}

	void NYRenderDevice::endSingleTimeCommandBuffers(vk::CommandBuffer commandBuffer){
		commandBuffer.end();
		vk::SubmitInfo submitInfo;
		submitInfo.commandBufferCount = 1;
		submitInfo.pCommandBuffers = &commandBuffer;
		
		graphicsQueue.submit(1, &submitInfo, VK_NULL_HANDLE);
		graphicsQueue.waitIdle();

		device.freeCommandBuffers(commandPool, 1, &commandBuffer);
	}

	void NYRenderDevice::createImage(VkImage& Image, const VkImageCreateInfo& ImageInfo, const VmaAllocationCreateInfo& AllocInfo, VmaAllocation& Allocation){
		auto result = vmaCreateImage(allocator, &ImageInfo, &AllocInfo, &Image, &Allocation, nullptr);
		NYLogger::checkAssert(result == VK_SUCCESS, "Failed to create image");
	}
}


