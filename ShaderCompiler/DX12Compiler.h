#pragma once
#include <include/Compiler.h>
#include <wrl.h>
#include <dxc/dxcapi.h>
using namespace Microsoft::WRL;
namespace ShaderCompiler {
	class DX12Compiler : public IShaderCompiler
	{
	public:
		DX12Compiler();
		virtual void AddInlcudePath(std::string const& path) override;
		virtual void AddMacro(std::string const& macro_name, std::string const& macro_value) override;
		virtual std::string PreprocessGLSLShader(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type) override;
		virtual std::vector<uint32_t> CompileGLSLShaderSource(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type, bool optimize = true) override;
	private:
		ComPtr<IDxcLibrary> p_Library;
		ComPtr<IDxcCompiler> p_Compiler;
	};

}