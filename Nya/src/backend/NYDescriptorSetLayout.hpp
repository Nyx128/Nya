#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"
#include "logging/NYLogger.hpp"

namespace Nya{
	class NYDescriptorSetLayout {
	public:
		NYDescriptorSetLayout(NYRenderDevice& _renderDevice);
		~NYDescriptorSetLayout();

		NYDescriptorSetLayout(NYDescriptorSetLayout const&) = delete;
		NYDescriptorSetLayout& operator=(NYDescriptorSetLayout const&) = delete;


		vk::DescriptorSetLayout& getLayout(){return layout;}
		void addBinding(vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags shaderStages);
		vk::DescriptorType getType(uint32_t bindingIndex);
		void buildLayout();
		std::vector<vk::DescriptorPoolSize>& getPoolSizes(uint32_t numSets);
		bool isBuilt() { return build; }
	private:

		void createPoolSize();

		NYRenderDevice& renderDevice;

		std::vector<vk::DescriptorSetLayoutBinding> bindings;
		vk::DescriptorSetLayoutCreateInfo layoutInfo;
		vk::DescriptorSetLayout layout;
		std::vector<vk::DescriptorPoolSize> poolSizes;

		uint32_t bindingIndex = 0;

		bool build = false;
	};
}
