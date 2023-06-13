#include "pch.hpp"
#include "NYDescriptorSetLayout.hpp"

namespace Nya {

	NYDescriptorSetLayout::NYDescriptorSetLayout(NYRenderDevice& _renderDevice):renderDevice(_renderDevice) {

	}

	NYDescriptorSetLayout::~NYDescriptorSetLayout(){
		renderDevice.getDevice().destroyDescriptorSetLayout(layout);
	}

	void NYDescriptorSetLayout::addBinding(vk::DescriptorType type, uint32_t count, vk::ShaderStageFlags shaderStages){
		NYLogger::checkAssert(!build, "Can't add binding after calling buildLayout()");

		vk::DescriptorSetLayoutBinding& binding = bindings.emplace_back();
		binding.descriptorType = type;
		binding.binding = bindingIndex;
		binding.descriptorCount = count;
		binding.stageFlags = shaderStages;

		bindingIndex++;
	}

	vk::DescriptorType NYDescriptorSetLayout::getType(uint32_t bindingIndex){
		NYLogger::checkAssert(bindingIndex < bindings.size(), "bindingIndex paramter in NYDescriptorSetLayout::getType() is out of range");
		return bindings[bindingIndex].descriptorType;
	}

	void NYDescriptorSetLayout::buildLayout(){
		NYLogger::checkAssert(!build, "Can't call buildLayout() more than once");

		createPoolSize();

		//make it so that we are able to change the descriptor data after it has been bound once, allowing for things like texture swapping
		std::vector<vk::DescriptorBindingFlags> bindingFlags;
		bindingFlags.resize(bindings.size());

		for (auto& flag : bindingFlags) {
			flag = vk::DescriptorBindingFlagBits::eUpdateAfterBind;
		}

		vk::DescriptorSetLayoutBindingFlagsCreateInfo flagsInfo;
		flagsInfo.bindingCount = bindings.size();
		flagsInfo.pBindingFlags = bindingFlags.data();

		vk::DescriptorSetLayoutCreateInfo createInfo{};
		createInfo.setBindings(bindings);
		createInfo.pNext = &flagsInfo;
		createInfo.flags = vk::DescriptorSetLayoutCreateFlagBits::eUpdateAfterBindPool;
		
		layout = renderDevice.getDevice().createDescriptorSetLayout(createInfo);
		build = true;
	}

	std::vector<vk::DescriptorPoolSize>& NYDescriptorSetLayout::getPoolSizes(uint32_t numSets) {
		NYLogger::checkAssert(build, "Can't get poolSizes before calling buildLayout()");
		for (int i = 0; i < bindings.size(); i++) {
			poolSizes[i].descriptorCount = bindings[i].descriptorCount * numSets;
			poolSizes[i].type = bindings[i].descriptorType;
		}
		return poolSizes;
	}

	void NYDescriptorSetLayout::createPoolSize(){
		poolSizes.resize(bindings.size());
		for (int i = 0; i < bindings.size();i++) {
			poolSizes[i].descriptorCount = bindings[i].descriptorCount * 3;
			poolSizes[i].type = bindings[i].descriptorType;
		}
	}
}