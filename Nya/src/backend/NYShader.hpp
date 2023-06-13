#pragma once
#include "pch.hpp"
#include "NYRenderDevice.hpp"
#include "NYDescriptorSetLayout.hpp"
#include "defines.hpp"

//this class will abstract
//->shader generation
//->shader modules
//->descriptor pool for each shader's resources
//->descriptor sets for shader resources
//->UBOs

namespace Nya {
	class NYShader {
	public:
		NYShader(NYRenderDevice& _renderDevice, std::string _vertexFilepath, std::string _fragmentFilepath);
		~NYShader();

		NYShader(NYShader const&) = delete;
		NYShader& operator=(NYShader const&) = delete;

		inline vk::ShaderModule& getVertexModule() { return vertexShaderModule; }
		inline vk::ShaderModule& getFragmentModule() { return fragmentShaderModule; }

	private:
		void compileShader();
		void readShader();
		void createModule();

		NYRenderDevice& renderDevice;

		std::string vertexFilepath;
		std::string fragmentFilepath;

		std::string vertexSpvPath;
		std::string fragmentSpvPath;

		vk::ShaderModule vertexShaderModule;
		vk::ShaderModule fragmentShaderModule;

		std::vector<char> vertexBinary;
		std::vector<char> fragmentBinary;
	};
}
