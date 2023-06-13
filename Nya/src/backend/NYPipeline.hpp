#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"
#include "NYSwapchain.hpp"
#include "game/NYSprite.hpp"
#include "NYDescriptorSetLayout.hpp"
#include "NYShader.hpp"
#include "NYRenderPass.hpp"

/*
This class is supposed to wrap the vk::Pipeline class
*/

namespace Nya {

	struct NYPipelineConfig {
		vk::PipelineVertexInputStateCreateInfo vertexInputStateInfo;
		vk::PipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
		vk::Viewport viewport;
		vk::Rect2D scissor;
		vk::PipelineRasterizationStateCreateInfo rasterizerStateInfo;
		vk::PipelineMultisampleStateCreateInfo multisampleStateInfo;
		vk::PipelineColorBlendAttachmentState colorBlendAttachment;
		vk::Format swapchainFormat;
	};

	class NYPipeline {
	public:
		NYPipeline(NYPipeline const&) = delete;
		NYPipeline& operator=(NYPipeline const&) = delete;


		//config must remain defined and valid until pipeline is created
		NYPipeline(NYRenderDevice& _renderDevice, NYPipelineConfig& _pipelineConfig, NYShader& _shader, NYDescriptorSetLayout& _descLayout, NYRenderPass& _renderPass);
		~NYPipeline();

		static void createDefaultPipelineConfig(NYPipelineConfig& config, NYSwapchain& swapchain,
			std::array<vk::VertexInputBindingDescription, 1>& bindingDesc, std::array<vk::VertexInputAttributeDescription, 2>& attribDesc);

		//getters
		vk::Pipeline& getPipeline() { return pipeline; }
		NYRenderPass& getRenderPass() { return renderPass; }
		vk::DescriptorSetLayout& getDescriptorSetLayout() { return descLayout.getLayout(); }
		vk::PipelineLayout& getLayout() { return pipelineLayout; }

	private:
		void createPipelineResources();
		void createPipeline();

		NYRenderDevice& renderDevice;

		NYPipelineConfig& pipelineConfig;
		vk::PipelineLayoutCreateInfo pipelineLayoutInfo;

		NYShader& shader;

		//pipeline creation resources
		vk::PipelineLayout pipelineLayout;
		vk::PipelineViewportStateCreateInfo viewportStateInfo;
		vk::PipelineColorBlendStateCreateInfo colorBlendStateInfo;
		std::array<vk::PipelineShaderStageCreateInfo, 2> shaderStages;


		vk::Pipeline pipeline;

		NYRenderPass& renderPass;
		vk::RenderPass offscreenPass;
		vk::RenderPass compositePass;

		NYDescriptorSetLayout& descLayout;
	};
}
