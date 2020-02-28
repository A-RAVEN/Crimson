#include "ShaderProcessor.h"
#include <fstream>
#include <iostream>
#include <array>
#include <regex>
#include <set>

using namespace ShaderCompiler;
ShaderProcessor::ShaderProcessor()
{
	p_Compiler = IShaderCompiler::GetCompiler();
	ResetShaderTypeMacros();
}
std::vector<uint32_t> ShaderProcessor::Compile(std::string const& file_name)
{
	ResetShaderTypeMacros();
	std::string ext = file_name.substr(file_name.find_last_of('.'));
	ShaderCompiler::ECompileShaderType shader_type = ECompileShaderType::E_SHADER_TYPE_MAX;
	if (ext == ".vert")
	{
		p_Compiler->AddMacro("VERTEX_SHADER", "1");
		shader_type = ECompileShaderType::E_SHADER_TYPE_VERTEX;
	}
	else if (ext == ".frag")
	{
		p_Compiler->AddMacro("FRAGMENT_SHADER", "1");
		shader_type = ECompileShaderType::E_SHADER_TYPE_FRAGMENT;
	}
	if (shader_type != ECompileShaderType::E_SHADER_TYPE_MAX)
	{
		std::ifstream shader_src(file_name);
		std::string src;
		if (shader_src.is_open())
		{
			std::string line;
			while (std::getline(shader_src, line))
			{
				src += line + "\n";
			}
			src = p_Compiler->PreprocessGLSLShader(file_name, src, shader_type);

			//std::cout << src << std::endl;
			return p_Compiler->CompileGLSLShaderSource(file_name, src, shader_type);
		}
		//switch (shader_type)
		//{
		//case ECompileShaderType::E_SHADER_TYPE_VERTEX:
		//	p_Compiler->AddMacro("VERTEX_SHADER", "0");
		//	break;
		//case ECompileShaderType::E_SHADER_TYPE_FRAGMENT:
		//	p_Compiler->AddMacro("FRAGMENT_SHADER", "0");
		//	break;
		//}
	}
	return std::vector<uint32_t>();
}

std::set<ECompileShaderType> FindCompileTockens(std::string const& src)
{
	//(?:\\s+(\\d+))
	std::regex pattern("^\\s*#pragma\\s+multicompile\\s+(.+)+$");
	std::smatch match_data;
	std::string remain_str = src;
	std::vector<std::string> token_vec;
	std::set<ECompileShaderType> return_val;
	while (regex_search(remain_str, match_data, pattern))
	{
		std::string tokens = match_data[1];
		std::string token("");
		for (char c : tokens)
		{
			if (c == ' ')
			{
				if (!token.empty())
				{
					token_vec.push_back(token);
					token = "";
				}
			}
			else
			{
				token += c;
			}
		}
		if (!token.empty())
		{
			token_vec.push_back(token);
			token = "";
		}
		for (auto& itr_tk : token_vec)
		{
			if (itr_tk == "vert")
			{
				return_val.insert(ECompileShaderType::E_SHADER_TYPE_VERTEX);
			}
			else if (itr_tk == "frag")
			{
				return_val.insert(ECompileShaderType::E_SHADER_TYPE_FRAGMENT);
			}
		}
		remain_str = match_data.suffix().str();
	}
	return return_val;
}

CompileResult ShaderProcessor::MultiCompile(std::string const& file_name)
{
	ResetShaderTypeMacros();
	std::array<std::string, static_cast<int>(ECompileShaderType::E_SHADER_TYPE_MAX)> macros =
	{
		"VERTEX_SHADER",
		"TESSCONTROL_SHADER",
		"TESSEVALUATION_SHADER",
		"GEOMETRY_SHADER",
		"FRAGMENT_SHADER",
		"COMPUTE_SHADER"
	};

	std::ifstream shader_src(file_name);
	std::string src;
	CompileResult compile_result;
	if (shader_src.is_open())
	{
		std::string line;
		while (std::getline(shader_src, line))
		{
			src += line + "\n";
		}
		auto tokens = FindCompileTockens(src);
		{
			ECompileShaderType last_type = ECompileShaderType::E_SHADER_TYPE_MAX;
			for (auto token : tokens)
			{
				if (last_type != ECompileShaderType::E_SHADER_TYPE_MAX)
				{
					p_Compiler->AddMacro(macros[static_cast<int>(last_type)], "0");
				}
				p_Compiler->AddMacro(macros[static_cast<int>(token)], "1");
				auto processed = p_Compiler->PreprocessGLSLShader(file_name, src, token);
				std::cout << processed << std::endl;
				auto binary = p_Compiler->CompileGLSLShaderSource(file_name, processed, token);
				if (!binary.empty())
				{
					compile_result.push_back(std::make_pair(static_cast<Crimson::EShaderType>(token), binary));
				}
				last_type = token;
			}
		}

		//for (uint32_t i = 0; i < 5; ++i)
		//{
		//	ECompileShaderType type = static_cast<ECompileShaderType>(i);
		//	if (i > 0)
		//	{
		//		p_Compiler->AddMacro(macros[i - 1], "0");
		//	}
		//	p_Compiler->AddMacro(macros[i], "1");

		//	auto processed = p_Compiler->PreprocessGLSLShader(file_name, src, type);
		//	std::cout << processed << std::endl;
		//	auto binary = p_Compiler->CompileGLSLShaderSource(file_name, processed, type);
		//	if (!binary.empty())
		//	{
		//		compile_result.push_back(std::make_pair(static_cast<Crimson::EShaderType>(i), binary));
		//	}
		//}
	}

	return compile_result;
}

void ShaderProcessor::ResetShaderTypeMacros()
{
	p_Compiler->AddMacro("VERTEX_SHADER", "0");
	p_Compiler->AddMacro("FRAGMENT_SHADER", "0");
}
