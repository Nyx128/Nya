#pragma once
#include "pch.hpp"
#include "backend/NYTexture.hpp"
#include "backend/NYDescriptorSetLayout.hpp"

namespace Nya {
	class NYSprite {
	public:
		struct Vertex {
			glm::vec3 position;
			glm::vec2 uv;
		};

		struct UniformObject {
			alignas(16) glm::mat4 model;
			alignas(16) glm::mat4 view;
			alignas(16) glm::mat4 proj;
		};

		struct PushData {
			glm::mat4 transformMatrix;
		};

		NYSprite(NYRenderDevice& _renderDevice, glm::vec3 _translation = glm::vec3(0.0f), glm::vec3 _scale = glm::vec3(1.0f), glm::vec3 _rotation = glm::vec3(0.0f));
		~NYSprite();

		void updateUniformBuffers(uint32_t frameIndex, UniformObject& ubo);
		void writeTexture(std::shared_ptr<NYTexture>& texture);
		vk::DescriptorSet& getDescriptorSet(uint32_t frameIndex) { return descriptorSets[frameIndex]; }

		static std::array<vk::VertexInputBindingDescription, 1> getBindingDescriptions() {
			std::array<vk::VertexInputBindingDescription, 1> bindingDescriptions;
			bindingDescriptions[0].binding = 0;
			bindingDescriptions[0].stride = sizeof(Vertex);
			bindingDescriptions[0].inputRate = vk::VertexInputRate::eVertex;
			return bindingDescriptions;
		}

		static std::array<vk::VertexInputAttributeDescription, 2> getAttributeDescriptions() {
			std::array<vk::VertexInputAttributeDescription, 2> attributeDescriptions;
			attributeDescriptions[0].binding = 0;
			attributeDescriptions[0].location = 0;
			attributeDescriptions[0].format = vk::Format::eR32G32B32Sfloat;
			attributeDescriptions[0].offset = offsetof(Vertex, position);

			attributeDescriptions[1].binding = 0;
			attributeDescriptions[1].location = 1;
			attributeDescriptions[1].format = vk::Format::eR32G32Sfloat;
			attributeDescriptions[1].offset = offsetof(Vertex, uv);

			return attributeDescriptions;
		}

		std::array<Vertex, 4>& getVertices() { return vertices; }
		std::array<uint32_t, 6>& getIndices() { return indices; }

		glm::vec3 translation;
		glm::vec3 scale;
		glm::vec3 rotation;

		glm::mat4& transform_matrix();

		NYRenderDevice& renderDevice;
		NYDescriptorSetLayout layout{ renderDevice };

		//rendering data
		vk::DescriptorPool descPool;
		std::vector<vk::DescriptorSet> descriptorSets;

		std::vector<VkBuffer> uniformBuffers;
		std::vector<VmaAllocation> uniformAllocations;
		std::vector<void*> mappedUniformMems;
		PushData pushData;
		UniformObject ubo;

		bool built = false;

	private:

		void createLayout();
		void allocateDescriptors();
		void createUniformBuffers();
		void initDescriptors();

		std::array<Vertex, 4> vertices = { Vertex{glm::vec3(0.5f,  0.5f, 0.0f), glm::vec2(1.0f, 1.0f)},
										   Vertex{glm::vec3(0.5f,  -0.5f, 0.0f), glm::vec2(1.0f, 0.0f)},
										   Vertex{glm::vec3(-0.5f, 0.5f, 0.0f), glm::vec2(0.0f, 1.0f)},
										   Vertex{glm::vec3(-0.5f, -0.5f, 0.0f), glm::vec2(0.0f, 0.0f)}};

		std::array <uint32_t, 6> indices = { 0,1,2,1,3,2 };
	};
}
