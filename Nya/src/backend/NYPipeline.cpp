#include "pch.hpp"
#include "NYPipeline.hpp"
#include "logging/NYLogger.hpp"
#include "NYRenderpass.hpp"

namespace Nya {
	NYPipeline::NYPipeline(NYRenderDevice& _renderDevice, NYPipelineConfig& _pipelineConfig, NYShader& _shader, NYDescriptorSetLayout& _descLayout, NYRenderPass& _renderPass)
		:renderDevice(_renderDevice), pipelineConfig(_pipelineConfig), shader(_shader), descLayout(_descLayout), renderPass(_renderPass) {
		NYLogger::checkAssert(descLayout.isBuilt(), "NYDescriptorSetLayout must be built before passing as parameter");
		createPipelineResources();
		createPipeline();
	}

	NYPipeline::~NYPipeline(){

		renderDevice.getDevice().destroyPipelineLayout(pipelineLayout);

		renderDevice.getDevice().destroyRenderPass(offscreenPass);
		renderDevice.getDevice().destroyRenderPass(compositePass);

		renderDevice.getDevice().destroyPipeline(pipeline);

		NYLogger::logTrace("NYPipeline destroyed");
	}

	void NYPipeline::createPipelineResources(){
		//make the shaderStages
		vk::PipelineShaderStageCreateInfo vertShaderStageInfo(vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eVertex,
			shader.getVertexModule(),
			"main");

		vk::PipelineShaderStageCreateInfo fragShaderStageInfo(vk::PipelineShaderStageCreateFlags(),
			vk::ShaderStageFlagBits::eFragment,
			shader.getFragmentModule(),
			"main");

		shaderStages[0] = vertShaderStageInfo;
		shaderStages[1] = fragShaderStageInfo;

		//gonna use the pipeline config to make viewport state
		viewportStateInfo = vk::PipelineViewportStateCreateInfo(vk::PipelineViewportStateCreateFlags(),
																									1,//no of viewports
																									&pipelineConfig.viewport,
																									1,//no of scissors
																									&pipelineConfig.scissor);

		//now using the color blend attachment from pipelineconfig, i will make the colorBlendState
		std::array<float, 4> blendConstants;
		blendConstants.fill(0.0f);
		colorBlendStateInfo = vk::PipelineColorBlendStateCreateInfo(vk::PipelineColorBlendStateCreateFlags(),
																										  false,//logic op enable
																										  vk::LogicOp::eCopy,//logic op
																										  1,//attachment count
																										  &pipelineConfig.colorBlendAttachment,
																										  blendConstants);

		vk::PushConstantRange range;
		range.offset = 0;
		range.size = sizeof(NYSprite::PushData);
		range.stageFlags = vk::ShaderStageFlagBits::eVertex;

		pipelineLayoutInfo = vk::PipelineLayoutCreateInfo(vk::PipelineLayoutCreateFlags(),
														  1,
														  &descLayout.getLayout(),//pSetLayouts
														  1,//pushConstantRangeCount
														  &range);//pPushConstantRanges

		//create the pipeline layout using its info from pipelineConfig
		pipelineLayout = renderDevice.getDevice().createPipelineLayout(pipelineLayoutInfo);

	}

	void NYPipeline::createPipeline(){
		vk::GraphicsPipelineCreateInfo pipelineInfo = vk::GraphicsPipelineCreateInfo(vk::PipelineCreateFlags(),//flags
																					2,//no of shader stages
																					shaderStages.data(),//stages
																					&pipelineConfig.vertexInputStateInfo,//vertex input state
																					&pipelineConfig.inputAssemblyInfo, //input assembly state
																					nullptr,						   //tesselation state
																					&viewportStateInfo,				   // viewport state
																					&pipelineConfig.rasterizerStateInfo,//rasterization state
																					&pipelineConfig.multisampleStateInfo,//multisample state
																					nullptr,							//depth stencil state
																					&colorBlendStateInfo,				//color blend state
																					nullptr,							//dynamic state
																					pipelineLayout,						//layout
																					renderPass.getRenderpass(),							//renderpass
																					0,									//subpass
																					nullptr,							//base pipeline handle
																					-1);								//base pipeline index

		pipeline = (renderDevice.getDevice().createGraphicsPipeline(nullptr, pipelineInfo)).value;
		NYLogger::logTrace("NYPipeline created");
	}

	void NYPipeline::createDefaultPipelineConfig(NYPipelineConfig& config, NYSwapchain& swapchain,
		std::array<vk::VertexInputBindingDescription, 1>& bindingDesc, std::array<vk::VertexInputAttributeDescription, 2>& attribDesc){
		//this will create config for default rendering

		config.vertexInputStateInfo = vk::PipelineVertexInputStateCreateInfo(vk::PipelineVertexInputStateCreateFlags(),
																			bindingDesc,//vertex binding descriptions
																			attribDesc);//vertex attribute descriptions

		config.inputAssemblyInfo = vk::PipelineInputAssemblyStateCreateInfo(vk::PipelineInputAssemblyStateCreateFlags(),
																			vk::PrimitiveTopology::eTriangleList, //topology
																		    false);//primitiveRestartEnable

		vk::Extent2D swapchainExtent = swapchain.getSwapchainExtent();
		
		config.viewport = vk::Viewport(0.0f, 0.0f, swapchainExtent.width, swapchainExtent.height, 0.0f, 1.0f);
		config.scissor = vk::Rect2D(vk::Offset2D(0.0f, 0.0f), swapchainExtent);

		config.rasterizerStateInfo = vk::PipelineRasterizationStateCreateInfo(vk::PipelineRasterizationStateCreateFlags(),
																			  false,//depthClampEnable
																			  false,//RasterizerDiscardEnable
																			  vk::PolygonMode::eFill,//polygonMode
																			  vk::CullModeFlagBits::eNone,//cullMode
																			  vk::FrontFace::eClockwise,//frontFace
																		      false,//depthBiasEnable
																			  0.0f,//depthBiasConstantFactor
																			  0.0f,//depthBiasClamp
																			  0.0f,//depthBiasSlopeFactor
																			  1.0f);//lineWidth

		config.multisampleStateInfo = vk::PipelineMultisampleStateCreateInfo(vk::PipelineMultisampleStateCreateFlags(),
																			 vk::SampleCountFlagBits::e1,//sample count
																			 false,//sample shading enable
																			 1.0f, //min sample shading
																			 nullptr, //pSampleMask
																			 false, //alphaToCoverageEnable
																			 false);//alphaToOneEnable

		config.colorBlendAttachment = vk::PipelineColorBlendAttachmentState(true,//blendEnable
																			vk::BlendFactor::eSrcAlpha,//srcColorBlendFactor
																			vk::BlendFactor::eOneMinusSrcAlpha,//dstColorBlendFactor
																			vk::BlendOp::eAdd,//colorBlendOp
																			vk::BlendFactor::eOne,//srcAlphaBlendFactor
																			vk::BlendFactor::eZero,//dstAlphaBlendFactor
																			vk::BlendOp::eAdd,//alphaBlendOp
																			vk::ColorComponentFlagBits::eR|
																			vk::ColorComponentFlagBits::eG|
																			vk::ColorComponentFlagBits::eB|
																			vk::ColorComponentFlagBits::eA);//colorWriteMask

		config.swapchainFormat = swapchain.getSwapchainFormat();
	}
}