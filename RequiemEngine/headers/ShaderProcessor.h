#pragma once
#include <Compiler.h>
#include <PipelineEnums.h>

using CompileResult = std::vector<std::pair<Crimson::EShaderType, std::vector<uint32_t>>>;
class ShaderProcessor
{
public:
	ShaderProcessor();
	std::vector<uint32_t> Compile(std::string const& file_name);
	CompileResult MultiCompile(std::string const& file_name);
private:
	void ResetShaderTypeMacros();
	ShaderCompiler::IShaderCompiler* p_Compiler;
};