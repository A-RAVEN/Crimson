#include "DX12Compiler.h"
//#include <dxc/Support/Unicode.h>
#include <iostream>
#include <Helper.h>
namespace ShaderCompiler {
	DX12Compiler::DX12Compiler()
	{
		HRESULT hr = DxcCreateInstance(CLSID_DxcLibrary, IID_PPV_ARGS(&p_Library));

		hr = DxcCreateInstance(CLSID_DxcCompiler, IID_PPV_ARGS(&p_Compiler));
	}
	void DX12Compiler::AddInlcudePath(std::string const& path)
	{
	}
	void DX12Compiler::AddMacro(std::string const& macro_name, std::string const& macro_value)
	{
	}
	std::string DX12Compiler::PreprocessGLSLShader(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type)
	{
		return std::string();
	}

	static std::wstring STATIC_SHADER_PROFILE_TABLE[static_cast<uint32_t>(ECompileShaderType::E_SHADER_TYPE_MAX)] =
	{
		L"vs_",
		L"hs_",
		L"ds_",
		L"gs_",
		L"ps_",
		L"cs_",
		//nvidia mesh shader
		L"as_",
		L"ms_",
		//nvidia ray tracing shader, all has "lib_" profile
		L"lib_",
		L"lib_",
		L"lib_",
		L"lib_",
		L"lib_",
		L"lib_",
	};

	std::wstring GetShaderProfile(ECompileShaderType shader_type, std::wstring shader_model)
	{
		return STATIC_SHADER_PROFILE_TABLE[static_cast<uint32_t>(shader_type)] + shader_model;
	}

	std::vector<uint32_t> DX12Compiler::CompileGLSLShaderSource(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type, bool optimize)
	{
		ComPtr<IDxcBlobEncoding> sourceBlob;
		p_Library->CreateBlobWithEncodingOnHeapCopy(shader_src.c_str(), shader_src.size(), CP_UTF8,
			&sourceBlob);

		std::wstring utf16FileName = Helper::StringToWideString(file_name);
		//Unicode::UTF8ToUTF16String(file_name.c_str(), &utf16FileName);

		ComPtr<IDxcOperationResult> compileResult;
		p_Compiler->Compile(sourceBlob.Get(), utf16FileName.c_str(), L"main", GetShaderProfile(shader_type, L"6_5").c_str(), nullptr, 0, nullptr, 0, nullptr, &compileResult);
		HRESULT status;
		compileResult->GetStatus(&status);
		if (FAILED(status))
		{
			
			//compileResult->GetErrorBuffer()
			std::cout << "D3D12 Compiler Error: Invalid Shader" << std::endl;
			return std::vector<uint32_t>();
		}
		ComPtr<IDxcBlob> program;
		compileResult->GetResult(&program);

		if (program != nullptr)
		{
			std::vector<uint32_t> result;
			result.resize(program->GetBufferSize() / sizeof(uint32_t));
			memcpy(result.data(), program->GetBufferPointer(), program->GetBufferSize());
			return result;
		}
		return std::vector<uint32_t>();
	}
}