#pragma once
#include <Compiler.h>
#include <PipelineEnums.h>
#include <Pipeline.h>

using CompileResult = std::vector<std::pair<Crimson::EShaderType, std::vector<uint32_t>>>;
using namespace Crimson;
class PipelineDescriptor
{
public:
	std::vector<BlendSetting> m_AlphaBlendSettings;
	std::vector<BlendSetting> m_ColorBlendSettings;
	std::vector<std::pair<uint32_t, std::string>> m_ExternalDescriptorSets;
	std::vector<std::string> m_VertexInputTypes;
	std::vector<std::pair<Crimson::EShaderType, std::vector<uint32_t>>> m_Shaders;
};

class ShaderProcessor
{
public:
	ShaderProcessor();
	std::vector<uint32_t> Compile(std::string const& file_name);
	
	CompileResult MultiCompile(std::string const& file_name);
	CompileResult MultiCompilePipeline(std::string const& file_name, PipelineDescriptor &descriptor);
private:
	void ResetShaderTypeMacros();
	ShaderCompiler::IShaderCompiler* p_Compiler;
};