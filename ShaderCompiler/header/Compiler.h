#pragma once
#include <string>
#include <vector>
#include <RenderInterface/header/Common.h>



namespace ShaderCompiler
{

	class BaseShaderParam
	{
	public:
		std::string name;
		TIndex set = INVALID_INDEX;
		TIndex binding = INVALID_INDEX;
	};

	class TextureParam : public BaseShaderParam
	{
	public:
		EShaderTextureType type;
		uint32_t subpassInputAttachmentID = INVALID_INDEX;
	};

	class BufferParam : public BaseShaderParam
	{
	public:
		EShaderBufferType type;
		size_t blockSize = 0;
	};

	class SamplerParam : public BaseShaderParam
	{
	public:
		EShaderSamplerType type;
	};

	class ShaderParams : public BaseShaderParam
	{
	public:
		std::vector<TextureParam> textures;
		std::vector<BufferParam> buffers;
		std::vector<SamplerParam> samplers;
	};

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
			, bool optimize = true
			, bool debug = false) = 0;

		virtual ShaderParams ExtractShaderParams(std::vector<uint32_t> const& spirv_source) = 0;
	private:
		static IShaderCompiler* m_Singleton;
	};
}