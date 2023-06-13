#include "pch.hpp"
#include "NYSprite.hpp"
#include "defines.hpp"

namespace Nya {
	NYSprite::NYSprite(NYRenderDevice& _renderDevice, glm::vec3 _translation, glm::vec3 _scale, glm::vec3 _rotation)
		:renderDevice(_renderDevice),translation(_translation), scale(_scale), rotation(_rotation) {
		createLayout();
		allocateDescriptors();
		createUniformBuffers();
		initDescriptors();
	}

	NYSprite::~NYSprite(){
		renderDevice.getDevice().destroyDescriptorPool(descPool);
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			vmaUnmapMemory(renderDevice.getAllocator(), uniformAllocations[i]);
			vmaDestroyBuffer(renderDevice.getAllocator(), uniformBuffers[i], uniformAllocations[i]);
		}
	}

	void NYSprite::updateUniformBuffers(uint32_t frameIndex, UniformObject& ubo) {
		memcpy(mappedUniformMems[frameIndex], &ubo, sizeof(ubo));

		VkDescriptorBufferInfo bufferInfo;
		bufferInfo.buffer = uniformBuffers[frameIndex];
		bufferInfo.offset = 0;
		bufferInfo.range = sizeof(UniformObject);

		VkWriteDescriptorSet write{};

		write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
		write.descriptorCount = 1;
		write.descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
		write.dstArrayElement = 0;
		write.dstBinding = 0;
		write.dstSet = descriptorSets[frameIndex];
		write.pBufferInfo = &bufferInfo;

		vkUpdateDescriptorSets(renderDevice.getDevice(), 1, &write, 0, nullptr);
	}

	void NYSprite::createLayout() {
		layout.addBinding(vk::DescriptorType::eUniformBuffer, 1, vk::ShaderStageFlagBits::eAllGraphics);
		layout.addBinding(vk::DescriptorType::eCombinedImageSampler, 1, vk::ShaderStageFlagBits::eAllGraphics);
		layout.buildLayout();
	}

	void NYSprite::allocateDescriptors() {
		//find a way to not do this

		vk::DescriptorPoolCreateInfo poolInfo;
		poolInfo.flags = vk::DescriptorPoolCreateFlagBits::eUpdateAfterBind;
		poolInfo.maxSets = MAX_FRAMES_IN_FLIGHT;
		poolInfo.setPoolSizes(layout.getPoolSizes(MAX_FRAMES_IN_FLIGHT));

		descPool = renderDevice.getDevice().createDescriptorPool(poolInfo);

		descriptorSets.resize(MAX_FRAMES_IN_FLIGHT);

		std::vector<vk::DescriptorSetLayout> layouts(MAX_FRAMES_IN_FLIGHT, layout.getLayout());
		vk::DescriptorSetAllocateInfo allocInfo;
		allocInfo.descriptorPool = descPool;
		allocInfo.descriptorSetCount = MAX_FRAMES_IN_FLIGHT;
		allocInfo.setSetLayouts(layouts);

		descriptorSets = renderDevice.getDevice().allocateDescriptorSets(allocInfo);
	}

	void NYSprite::createUniformBuffers() {
		uniformBuffers.resize(MAX_FRAMES_IN_FLIGHT);
		uniformAllocations.resize(MAX_FRAMES_IN_FLIGHT);
		mappedUniformMems.resize(MAX_FRAMES_IN_FLIGHT);

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_CPU_ONLY;

		std::array<uint32_t, 1> queueFamilyIndices = { renderDevice.getGraphicsQueueFamilyIndex() };

		vk::BufferCreateInfo bufferInfo;
		bufferInfo.setQueueFamilyIndices(queueFamilyIndices);
		bufferInfo.usage = vk::BufferUsageFlagBits::eUniformBuffer;
		bufferInfo.size = sizeof(UniformObject);
		bufferInfo.setSharingMode(vk::SharingMode::eExclusive);

		VkBufferCreateInfo buffInfo = static_cast<VkBufferCreateInfo>(bufferInfo);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			auto result = vmaCreateBuffer(renderDevice.getAllocator(), &buffInfo, &allocInfo, &uniformBuffers[i], &uniformAllocations[i], nullptr);
			NYLogger::checkAssert(result == VK_SUCCESS, "failed to create uniform buffers");

			vmaMapMemory(renderDevice.getAllocator(), uniformAllocations[i], &mappedUniformMems[i]);
		}
	}

	void NYSprite::writeTexture(std::shared_ptr<NYTexture>& texture) {
		VkDescriptorImageInfo imageInfo;
		imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
		imageInfo.sampler = texture->getSampler();
		imageInfo.imageView = texture->getImageView();

		VkWriteDescriptorSet write{};
		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
			write.sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET;
			write.descriptorCount = 1;
			write.descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
			write.dstArrayElement = 0;
			write.dstBinding = 1;
			write.dstSet = descriptorSets[i];
			write.pImageInfo = &imageInfo;

			vkUpdateDescriptorSets(renderDevice.getDevice(), 1, &write, 0, nullptr);
		}
	}

	void NYSprite::initDescriptors() {
		UniformObject ubo;
		ubo.model = glm::mat4(1.0f);
		ubo.view = glm::mat4(1.0f);
		ubo.proj = glm::mat4(1.0f);

		for (int i = 0; i < MAX_FRAMES_IN_FLIGHT;i++) {
			updateUniformBuffers(i, ubo);
		}
	}

	glm::mat4& NYSprite::transform_matrix() {
		//thanks to https://www.youtube.com/@BrendanGalea for the simplified matrix calculations
		const float c3 = glm::cos(rotation.z);
		const float s3 = glm::sin(rotation.z);
		const float c2 = glm::cos(rotation.x);
		const float s2 = glm::sin(rotation.x);
		const float c1 = glm::cos(rotation.y);
		const float s1 = glm::sin(rotation.y);
		glm::mat4 matrix = {
			{
				scale.x * (c1 * c3 + s1 * s2 * s3),
				scale.x * (c2 * s3),
				scale.x * (c1 * s2 * s3 - c3 * s1),
				0.0f,
			},
			{
				scale.y * (c3 * s1 * s2 - c1 * s3),
				scale.y * (c2 * c3),
				scale.y * (c1 * c3 * s2 + s1 * s3),
				0.0f,
			},
			{
				scale.z * (c2 * s1),
				scale.z * (-s2),
				scale.z * (c1 * c2),
				0.0f,
			},
			{translation.x, translation.y, translation.z, 1.0f}
		};

		return matrix;
	}
}