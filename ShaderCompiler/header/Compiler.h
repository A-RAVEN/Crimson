#pragma once
#include <string>
#include <vector>
#include <RenderInterface/header/Common.h>



namespace ShaderCompiler
{
	class IShaderCompiler
	{
	public:
		virtual ~IShaderCompiler() = default;
		virtual void AddInlcudePath(std::string const& path) = 0;
		virtual void AddMacro(std::string const& macro_name, std::string const& macro_value) = 0;
		virtual std::string PreprocessShader(
			EShaderSourceType shader_source_type
			, std::string const& file_name
			, std::string const& shader_src
			, ECompileShaderType shader_type) = 0;
		virtual std::vector<uint32_t> CompileShaderSource(
			EShaderSourceType shader_source_type
			, std::string const& file_name
			, std::string const& shader_src
			, std::string const& entry_point
			, ECompileShaderType shader_type
			, bool optimize = true) = 0;
	private:
		static IShaderCompiler* m_Singleton;
	};
}