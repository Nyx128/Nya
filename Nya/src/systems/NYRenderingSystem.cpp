#include "pch.hpp"
#include "NYRenderingSystem.hpp"
#include "logging/NYLogger.hpp"
#include "utils/NYTimer.hpp"

namespace Nya {
	NYRenderingSystem::NYRenderingSystem(NYRenderer& _renderer, NYRenderDevice& _renderDevice, std::vector<std::unique_ptr<NYSprite>>& _sprites):
		renderer(_renderer), renderDevice(_renderDevice) {
		load(_sprites);
	}

	NYRenderingSystem::~NYRenderingSystem(){
		for (int i = 0; i < vertexBuffers.size(); i++) {
			vmaDestroyBuffer(renderDevice.getAllocator(), vertexBuffers[i], vertexBufferAllocations[i]);
			vmaDestroyBuffer(renderDevice.getAllocator(), indexBuffers[i], indexBufferAllocations[i]);
		}

	}

	void NYRenderingSystem::load(std::vector<std::unique_ptr<NYSprite>>& _sprites){

		NYTimer timer;
		vertexBuffers.clear();
		indexBuffers.clear();

		vertexBuffers.resize(_sprites.size());
		vertexBufferAllocations.resize(_sprites.size());

		indexBuffers.resize(_sprites.size());
		indexBufferAllocations.resize(_sprites.size());

		VmaAllocationCreateInfo allocInfo{};
		allocInfo.usage = VMA_MEMORY_USAGE_GPU_ONLY;
		
		std::array<uint32_t, 1> queueFamilyIndices = { renderDevice.getGraphicsQueueFamilyIndex() };

		size_t vertexBuffSize = sizeof(NYSprite::Vertex) * _sprites[0]->getVertices().size();

		vk::BufferCreateInfo vertexBufferInfo;
		vertexBufferInfo.setQueueFamilyIndices(queueFamilyIndices);
		vertexBufferInfo.usage = vk::BufferUsageFlagBits::eVertexBuffer;
		vertexBufferInfo.size = vertexBuffSize;
		vertexBufferInfo.sharingMode = vk::SharingMode::eExclusive;

		size_t indexBuffSize = sizeof(uint32_t) * _sprites[0]->getIndices().size();

		vk::BufferCreateInfo indexBufferInfo;
		indexBufferInfo.setQueueFamilyIndices(queueFamilyIndices);
		indexBufferInfo.usage = vk::BufferUsageFlagBits::eIndexBuffer;
		indexBufferInfo.size = indexBuffSize;
		indexBufferInfo.sharingMode = vk::SharingMode::eExclusive;

		auto vertBuffInfo = static_cast<VkBufferCreateInfo>(vertexBufferInfo);
		auto indexBuffInfo = static_cast<VkBufferCreateInfo>(indexBufferInfo);

		for (int i = 0; i < _sprites.size(); i++) {

			auto result = vmaCreateBuffer(renderDevice.getAllocator(), &vertBuffInfo, &allocInfo, &vertexBuffers[i], &vertexBufferAllocations[i], nullptr);

			NYLogger::checkAssert(result == VK_SUCCESS, "failed to create vertex buffer");

			void* mappedMem;
			vmaMapMemory(renderDevice.getAllocator(), vertexBufferAllocations[i], &mappedMem);
			memcpy(mappedMem, _sprites[i]->getVertices().data(), vertexBuffSize);

			vmaUnmapMemory(renderDevice.getAllocator(), vertexBufferAllocations[i]);

			result = vmaCreateBuffer(renderDevice.getAllocator(), &indexBuffInfo, &allocInfo, &indexBuffers[i], &indexBufferAllocations[i], nullptr);

			NYLogger::checkAssert(result == VK_SUCCESS, "failed to create index buffer");

			vmaMapMemory(renderDevice.getAllocator(), indexBufferAllocations[i], &mappedMem);
			memcpy(mappedMem, _sprites[i]->getIndices().data(), indexBuffSize);

			vmaUnmapMemory(renderDevice.getAllocator(), indexBufferAllocations[i]);

		}
		timer.endTimer();
		std::cout << timer.getMillis() << std::endl;
		
	}

	void NYRenderingSystem::render(std::vector<std::unique_ptr<NYSprite>>& _sprites, NYPipeline& pipeline) {
		float aspect_ratio = 16.0f / 9.0f;
		renderer.beginRenderPass(glm::vec4(0.05f, 0.05f, 0.05f, 0.05f), pipeline);
		renderer.bindPipeline(pipeline);
		for (int i = 0; i < vertexBuffers.size();i++) {
			
			NYSprite::UniformObject ubo;
			ubo.model = _sprites[i]->transform_matrix();
			ubo.proj = glm::mat4(0.0f);
			ubo.proj = glm::ortho(-5.0f, 5.0f, -5.0f/aspect_ratio, 5.0f/aspect_ratio);
			ubo.view = glm::mat4(1.0f);
			
			_sprites[i]->updateUniformBuffers(renderer.getFrameIndex(), ubo);
			_sprites[i]->pushData.transformMatrix = ubo.proj * ubo.model;
			renderer.pushConstants(pipeline, &_sprites[i]->pushData, sizeof(NYSprite::PushData));
			renderer.draw(vertexBuffers[i], indexBuffers[i], pipeline, _sprites[i]->getIndices().size(), _sprites[i]->getDescriptorSet(renderer.getFrameIndex()));
		}
		renderer.endRenderPass(pipeline);
	}
}