#pragma once
#include <string>
#include <vector>

namespace ShaderCompiler
{
	enum class ECompileShaderType : uint8_t
	{
		E_SHADER_TYPE_VERTEX = 0,
		E_SHADER_TYPE_TESSCTR,
		E_SHADER_TYPE_TESSEVL,
		E_SHADER_TYPE_GEOMETRY,
		E_SHADER_TYPE_FRAGMENT,
		E_SHADER_TYPE_COMPUTE,
		//nvidia mesh shader
		E_SHADER_TYPE_TASK_NV,
		E_SHADER_TYPE_MESH_NV,
		E_SHADER_TYPE_MAX,
	};

	class IShaderCompiler
	{
	public:
		virtual void AddInlcudePath(std::string const& path) = 0;
		virtual void AddMacro(std::string const& macro_name, std::string const& macro_value) = 0;
		virtual std::string PreprocessGLSLShader(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type) = 0;
		virtual std::vector<uint32_t> CompileGLSLShaderSource(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type, bool optimize = true) = 0;
		static IShaderCompiler* GetCompiler();
		static void DisposeCompiler();
	private:
		static IShaderCompiler* m_Singleton;
	};
}