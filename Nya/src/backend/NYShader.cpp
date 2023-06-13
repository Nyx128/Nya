#include "pch.hpp"
#include "NYShader.hpp"
#include "logging/NYLogger.hpp"
#include <sstream>

namespace Nya {
	NYShader::NYShader(NYRenderDevice& _renderDevice, std::string _vertexFilepath, std::string _fragmentFilepath)
		:renderDevice(_renderDevice), fragmentFilepath(_fragmentFilepath), vertexFilepath(_vertexFilepath) {
		compileShader();
		readShader();
		createModule();
	}

	NYShader::~NYShader(){
		renderDevice.getDevice().destroyShaderModule(vertexShaderModule);
		renderDevice.getDevice().destroyShaderModule(fragmentShaderModule);
	}

	void NYShader::compileShader(){
		//compile shader
		std::filesystem::path workingDir = std::filesystem::current_path();
		std::string shaderDir = workingDir.generic_string() + "/src/shaders";

		std::filesystem::path shaderPath(shaderDir);

		//kind of pointless but just in case
		if (!std::filesystem::exists(shaderPath)) {
			std::filesystem::create_directory(shaderPath);
		}

		//vertex shader
		std::stringstream vertexStream(vertexFilepath);
		std::string segment;
		std::vector<std::string> segments;

		while (std::getline(vertexStream, segment, '/')) {
			segments.push_back(segment);
		}

		std::string shaderName = segments.back();

		vertexSpvPath = shaderDir + "/" + shaderName + ".spv";

		std::string command = shaderDir + "/glslc.exe " + shaderDir + "/" + shaderName + " -o " + vertexSpvPath;
		system(command.c_str());

		segments.clear();
		segment.clear();
		//fragment shader
		std::stringstream fragStream(fragmentFilepath);

		while (std::getline(fragStream, segment, '/')) {
			segments.push_back(segment);
		}

		shaderName = segments.back();

		fragmentSpvPath = shaderDir + "/" + shaderName + ".spv";

		command = shaderDir + "/glslc.exe " + shaderDir + "/" + shaderName + " -o " + fragmentSpvPath;
		system(command.c_str());
	}

	void NYShader::readShader(){
		std::ifstream vertFile(vertexSpvPath, std::ios::ate | std::ios::binary);
		NYLogger::checkAssert(vertFile.is_open(), "Failed to read shader");

		size_t fileSize = vertFile.tellg();
		vertexBinary.resize(fileSize);
		vertFile.seekg(0);
		vertFile.read(vertexBinary.data(), fileSize);

		vertFile.close();

		std::ifstream fragFile(fragmentSpvPath, std::ios::ate | std::ios::binary);
		NYLogger::checkAssert(fragFile.is_open(), "Failed to read shader");

		fileSize = fragFile.tellg();
		fragmentBinary.resize(fileSize);
		fragFile.seekg(0);
		fragFile.read(fragmentBinary.data(), fileSize);

		fragFile.close();
	}

	void NYShader::createModule(){
		VkShaderModuleCreateInfo vertexShaderModuleInfo{};
		vertexShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		vertexShaderModuleInfo.codeSize = vertexBinary.size();
		vertexShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(vertexBinary.data());

		vk::ShaderModuleCreateInfo vertexModuleInfo = static_cast<vk::ShaderModuleCreateInfo>(vertexShaderModuleInfo);

		vertexShaderModule = renderDevice.getDevice().createShaderModule(vertexModuleInfo);

		VkShaderModuleCreateInfo fragmentShaderModuleInfo{};
		fragmentShaderModuleInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
		fragmentShaderModuleInfo.codeSize = fragmentBinary.size();
		fragmentShaderModuleInfo.pCode = reinterpret_cast<uint32_t*>(fragmentBinary.data());

		vk::ShaderModuleCreateInfo fragmentModuleInfo = static_cast<vk::ShaderModuleCreateInfo>(fragmentShaderModuleInfo);

		fragmentShaderModule = renderDevice.getDevice().createShaderModule(fragmentModuleInfo);
	}
}