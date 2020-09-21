#include "ShaderProcessor.h"
#include <fstream>
#include <iostream>
#include <array>
#include <regex>
#include <set>
#include <map>
#include <Pipeline.h>
#include <external/rapidjson/reader.h>
#include <external/rapidjson/document.h>

using namespace ShaderCompiler;
using namespace rapidjson;
using namespace Crimson;

std::map<std::string, ECompileShaderType> ShaderTypeMap =
{
	{"vert", ECompileShaderType::E_SHADER_TYPE_VERTEX},
	{"tesc", ECompileShaderType::E_SHADER_TYPE_TESSCTR},
	{"tese", ECompileShaderType::E_SHADER_TYPE_TESSEVL},
	{"geom", ECompileShaderType::E_SHADER_TYPE_GEOMETRY},
	{"frag", ECompileShaderType::E_SHADER_TYPE_FRAGMENT},
	{"comp", ECompileShaderType::E_SHADER_TYPE_COMPUTE},
	{"task", ECompileShaderType::E_SHADER_TYPE_TASK_NV},
	{"mesh", ECompileShaderType::E_SHADER_TYPE_MESH_NV},
	{"rgen", ECompileShaderType::E_SHADER_TYPE_RAYGEN_NV},
	{"rahit", ECompileShaderType::E_SHADER_TYPE_ANYHIT_NV},
	{"rchit", ECompileShaderType::E_SHADER_TYPE_CLOSEHIT_NV},
	{"rmiss", ECompileShaderType::E_SHADER_TYPE_MISS_NV},
	{"rint", ECompileShaderType::E_SHADER_TYPE_INTERSECTION_NV},
	{"rcall", ECompileShaderType::E_SHADER_TYPE_CALLABLE_NV},
};

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
	}
	return std::vector<uint32_t>();
}

std::set<ECompileShaderType> FindCompileTockens(std::string const& src)
{
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
			return_val.insert(ShaderTypeMap[itr_tk]);
		}
		remain_str = match_data.suffix().str();
	}
	return return_val;
}

static std::map<std::string, EBlendFactor> BLEND_FACTOR_TABLE =
{
	std::make_pair("SRC_ALPHA", EBlendFactor::E_SRC_ALPHA),
	std::make_pair("ONE_MINUS_SRC_ALPHA", EBlendFactor::E_ONE_MINUS_SRC_ALPHA),
	std::make_pair("DST_ALPHA", EBlendFactor::E_DST_ALPHA),
	std::make_pair("ONE_MINUS_DST_ALPHA", EBlendFactor::E_ONE_MINUS_DST_ALPHA),
	std::make_pair("ONE", EBlendFactor::E_ONE),
	std::make_pair("ZERO", EBlendFactor::E_ZERO),
};

static std::map<std::string, EBlendOp> BLEND_OP_TABLE =
{
	std::make_pair("ADD", EBlendOp::E_ADD),
	std::make_pair("MINUS", EBlendOp::E_MINUS),
	std::make_pair("SUBTRACT", EBlendOp::E_SUBTRACT),
	std::make_pair("MULTIPLY", EBlendOp::E_MULTIPLY),
};

static std::map<std::string, std::vector<EDataType>> DATA_TYPE_TABLE =
{
	std::make_pair(std::string("TRANSFORM_ID"), std::vector<EDataType>{EDataType::EUINT})
};


void ParsePipelineSettings(std::string const& setting_string, PipelineDescriptor &pipeline)
{
	Document doc;
	doc.Parse(setting_string.c_str());
	auto itr_member = doc.MemberBegin();
	while (itr_member != doc.MemberEnd())
	{
		std::string key(itr_member->name.GetString());
		auto& val = itr_member->value;
		if (key == "color_blend" && val.IsObject())
		{
			auto src_factor = val["src_factor"].GetArray();
			auto dst_factor = val["dst_factor"].GetArray();
			auto operation = val["operation"].GetArray();
			int count = src_factor.Size();
			for (int i = 0; i < count; ++i)
			{
				BlendSetting new_setting{};
				std::string src_factor(src_factor[i].GetString());
				if (src_factor == "NO_BLEND")
				{
					//new_setting.IsNoBlendSetting = true;
				}
				else
				{
					//new_setting.IsNoBlendSetting = false;
					new_setting.m_SrcFactor = BLEND_FACTOR_TABLE[src_factor];
					new_setting.m_DstFactor = BLEND_FACTOR_TABLE[dst_factor[i].GetString()];
					new_setting.m_BlendOp = BLEND_OP_TABLE[operation[i].GetString()];
				}
				pipeline.m_ColorBlendSettings.push_back(new_setting);
			}
		}
		else if (key == "alpha_blend" && val.IsObject())
		{
			auto src_factor = val["src_factor"].GetArray();
			auto dst_factor = val["dst_factor"].GetArray();
			auto operation = val["operation"].GetArray();
			int count = src_factor.Size();
			for (int i = 0; i < count; ++i)
			{
				BlendSetting new_setting{};
				std::string src_factor(src_factor[i].GetString());
				new_setting.m_SrcFactor = BLEND_FACTOR_TABLE[src_factor];
				new_setting.m_DstFactor = BLEND_FACTOR_TABLE[dst_factor[i].GetString()];
				new_setting.m_BlendOp = BLEND_OP_TABLE[operation[i].GetString()];
				pipeline.m_AlphaBlendSettings.push_back(new_setting);
			}
		}
		else if (key == "vertex_type" && val.IsArray())
		{
			auto str_arr = val.GetArray();
			int count = str_arr.Size();
			for (int i = 0; i < count; ++i)
			{
				pipeline.m_VertexInputTypes.push_back(str_arr[i].GetString());
			}
		}
		else if (key == "set_reference" && val.IsObject())
		{
			auto itr_token = val.MemberBegin();
			while (itr_token != val.MemberEnd())
			{
				std::string name = itr_token->name.GetString();
				uint32_t id = itr_token->value.GetInt();
				pipeline.m_ExternalDescriptorSets.push_back(std::make_pair(id, name));
				++itr_token;
			}
		}
		++itr_member;
	}
}

std::string FindPipelineSettings(std::string const& src, PipelineDescriptor &descriptor)
{
	std::string return_val = src;
	std::regex pattern("^\\s*PIPELINE_CONFIG\\s*^([\\s\\S\\character]+)^\\s*PIPELINE_CONFIG\\s*$");
	//std::regex pattern("^\\s*#if PIPELINE_CONFIG([\s\S\character]+)#endif");
	std::smatch match_data;
	if (regex_search(src, match_data, pattern))
	{
		std::string str = match_data[1].str();
		ParsePipelineSettings(str, descriptor);
		return_val = match_data.prefix().str() + "\n#if PIPELINE_CONFIG\n" + match_data[1].str() + "#endif\n" + match_data.suffix().str();
	}
	std::cout << return_val << std::endl;
	return return_val;
}

CompileResult ShaderProcessor::MultiCompile(std::string const& file_name)
{
	//ResetShaderTypeMacros();
	std::array<std::string, static_cast<int>(ECompileShaderType::E_SHADER_TYPE_MAX)> macros =
	{
		"VERTEX_SHADER",
		"TESSCONTROL_SHADER",
		"TESSEVALUATION_SHADER",
		"GEOMETRY_SHADER",
		"FRAGMENT_SHADER",
		"COMPUTE_SHADER",
		"TASK_SHADER",
		"MESH_SHADER",
		"RAYGEN_SHADER",
		"ANYHIT_SHADER",
		"CLOSEHIT_SHADER",
		"MISS_SHADER",
		"INTERSECTION_SHADER",
		"CALLABLE_SHADER"
	};
	for (auto& shader_macro : macros)
	{
		p_Compiler->AddMacro(shader_macro, "0");
	}
	p_Compiler->AddMacro("PIPELINE_CONFIG", "0");

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
		PipelineDescriptor desc;
		src = FindPipelineSettings(src, desc);
		auto tokens = FindCompileTockens(src);
		if (tokens.empty())
		{
			
		}
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
	}

	return compile_result;
}

CompileResult ShaderProcessor::MultiCompilePipeline(std::string const& file_name, PipelineDescriptor& descriptor)
{
	std::array<std::string, static_cast<int>(ECompileShaderType::E_SHADER_TYPE_MAX)> macros =
	{
		"VERTEX_SHADER",
		"TESSCONTROL_SHADER",
		"TESSEVALUATION_SHADER",
		"GEOMETRY_SHADER",
		"FRAGMENT_SHADER",
		"COMPUTE_SHADER",
		"TASK_SHADER",
		"MESH_SHADER",
		"RAYGEN_SHADER",
		"ANYHIT_SHADER",
		"CLOSEHIT_SHADER",
		"MISS_SHADER",
		"INTERSECTION_SHADER",
		"CALLABLE_SHADER"
	};
	for (auto& shader_macro : macros)
	{
		p_Compiler->AddMacro(shader_macro, "0");
	}
	p_Compiler->AddMacro("PIPELINE_CONFIG", "0");

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
		src = FindPipelineSettings(src, descriptor);
		auto tokens = FindCompileTockens(src);
		if (tokens.empty())
		{

		}
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
	}
	return compile_result;
}

void ShaderProcessor::ResetShaderTypeMacros()
{
	p_Compiler->AddMacro("VERTEX_SHADER", "0");
	p_Compiler->AddMacro("FRAGMENT_SHADER", "0");
}
