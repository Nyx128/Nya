#pragma once
#include "pch.hpp"
#include "backend/NYRenderer.hpp"
#include "game/NYSprite.hpp"
#include "backend/NYPipeline.hpp"

//system that utilizes the NYRenderer and deals with all the pre-rendering stuff like creating buffers, descriptors, etc
namespace Nya {
	class NYRenderingSystem {
	public:
		NYRenderingSystem(NYRenderer& _renderer, NYRenderDevice& _renderDevice, std::vector<std::unique_ptr<NYSprite>>& _sprites);
		~NYRenderingSystem();

		void load(std::vector<std::unique_ptr<NYSprite>>& _sprites);
		void render(std::vector<std::unique_ptr<NYSprite>>& _sprites, NYPipeline& pipeline);

	private:
		NYRenderer& renderer;
		NYRenderDevice& renderDevice;

		std::vector<VkBuffer> vertexBuffers;
		std::vector<VmaAllocation> vertexBufferAllocations;

		std::vector<VkBuffer> indexBuffers;
		std::vector<VmaAllocation> indexBufferAllocations;
	};
}
