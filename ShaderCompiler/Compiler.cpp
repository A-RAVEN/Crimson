#ifdef _WIN32
#define NOMINMAX
#include <Windows.h>
#endif
#define NV_EXTENSIONS
#include <header/Compiler.h>
#include <FileIncluder.h>
#include <shaderc/shaderc.hpp>
#include <iostream>
#include <SharedTools/header/LibraryExportCommon.h>
#include <spirv_cross/spirv_cross.hpp>


namespace ShaderCompiler
{
	IShaderCompiler* IShaderCompiler::m_Singleton = nullptr;

	static shaderc_shader_kind SHADER_KIND_TABLE[] =
	{
		shaderc_glsl_default_vertex_shader,
		shaderc_glsl_default_tess_control_shader,
		shaderc_glsl_default_tess_evaluation_shader,
		shaderc_glsl_default_geometry_shader,
		shaderc_glsl_default_fragment_shader,
		shaderc_glsl_default_compute_shader,

		//nvidia mesh shader
		shaderc_task_shader,
		shaderc_mesh_shader,

		//nvidia raytracing shader
		shaderc_raygen_shader,
		shaderc_anyhit_shader,
		shaderc_closesthit_shader,
		shaderc_miss_shader,
		shaderc_intersection_shader,
		shaderc_callable_shader,
	};

	static shaderc_source_language SOURCE_LANGUAGE_TABLE[] =
	{
		shaderc_source_language_hlsl,
		shaderc_source_language_glsl
	};

	class Compiler_Impl : public IShaderCompiler
	{
	public:
		Compiler_Impl()
		{
			m_Options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_3);
			//m_Options.SetTargetSpirv(shaderc_spirv_version_1_6);
			m_Options.SetIncluder(std::unique_ptr <shaderc::CompileOptions::IncluderInterface> (new FileIncluder()));
		}
		~Compiler_Impl()
		{

		}
		virtual void AddInlcudePath(std::string const& path) override
		{
//			if (!m_Includer.AddIncludePath(path))
//			{
//#ifdef _WIN32
//				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
//				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
//#endif
//				std::cerr << "Shader Compiler Add Path:\n" << path << "\nFailed!" << std::endl;
//#ifdef _WIN32
//				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
//#endif
//			}
		}
		virtual void AddMacro(std::string const& macro_name, std::string const& macro_value) override
		{
			if (macro_value == "") {
				m_Options.AddMacroDefinition(macro_name);
			}
			else
			{
				m_Options.AddMacroDefinition(macro_name, macro_value);
			}
		}
		virtual std::string PreprocessShader(
			EShaderSourceType shader_source_type
			, std::string const& file_name
			, std::string const& shader_src
			, ECompileShaderType shader_type) override
		{

			shaderc_source_language source_language = SOURCE_LANGUAGE_TABLE[static_cast<uint32_t>(shader_source_type)];

			shaderc::PreprocessedSourceCompilationResult result = 
				m_Compiler.PreprocessGlsl(shader_src, SHADER_KIND_TABLE[static_cast<uint32_t>(shader_type)], file_name.c_str(), m_Options);
			
			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
#ifdef _WIN32
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
				std::cerr << "Shader Processor Error: \n" << result.GetErrorMessage() << std::endl;
#ifdef _WIN32
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
				return "";
			}

			return { result.cbegin(), result.cend() };
		}
		virtual std::vector<uint32_t> CompileShaderSource(
			EShaderSourceType shader_source_type
			, std::string const& file_name
			, std::string const& shader_src
			, std::string const& entry_point
			, ECompileShaderType shader_type
			, bool optimize
			, bool debug) override
		{
			if (optimize)
			{
				m_Options.SetOptimizationLevel(shaderc_optimization_level_performance);
			}
			else
			{
				m_Options.SetOptimizationLevel(shaderc_optimization_level_zero);
			}
			if (debug)
			{
				m_Options.SetGenerateDebugInfo();
			}
			shaderc_source_language source_language = SOURCE_LANGUAGE_TABLE[static_cast<uint32_t>(shader_source_type)];

			m_Options.SetSourceLanguage(source_language);

			shaderc::SpvCompilationResult result = m_Compiler.CompileGlslToSpv(shader_src.data(),
				shader_src.size(),
				SHADER_KIND_TABLE[static_cast<uint32_t>(shader_type)],
				file_name.data(),
				entry_point.c_str(),
				m_Options);

			if (result.GetCompilationStatus() != shaderc_compilation_status_success) {
				if (result.GetCompilationStatus() != shaderc_compilation_status_null_result_object)
				{
#ifdef _WIN32
					HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
					SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
					//std::cerr << result.GetCompilationStatus() << std::endl;
					std::cerr << "Shader Processor Error: \n" << result.GetErrorMessage() << std::endl;
#ifdef _WIN32
					SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
				}
				return std::vector<uint32_t>();
			}

			return { result.cbegin(), result.cend() };
		}

		void ExtractBaseShaderParamInfo(
			BaseShaderParam& inoutParam
			, spirv_cross::Resource const& resource
			, spirv_cross::Compiler& compiler)
		{
			inoutParam.name = resource.name;
			inoutParam.set = compiler.get_decoration(resource.id, spv::DecorationDescriptorSet);
			inoutParam.binding = compiler.get_decoration(resource.id, spv::DecorationBinding);
		}

#pragma region Texture Params

		TextureParam ExtractTextureParam(spirv_cross::Resource const& image
			, spirv_cross::Compiler& compiler)
		{
			TextureParam textureParam{};
			ExtractBaseShaderParamInfo(textureParam, image, compiler);
			const spirv_cross::SPIRType& type = compiler.get_type(image.base_type_id);
			switch (type.image.dim)
			{
			case spv::Dim::Dim1D:
				textureParam.type = 
					EShaderTextureType::eTexture1D;
				break;
			case spv::Dim::Dim2D:
				textureParam.type = 
					EShaderTextureType::eTexture2D;
				break;
			case spv::Dim::Dim3D:
				textureParam.type = 
					EShaderTextureType::eTexture3D;
				break;
			case spv::Dim::DimBuffer:
				textureParam.type =
					EShaderTextureType::eTexelBuffer;
				break;
			case spv::Dim::DimCube:
				textureParam.type = 
					EShaderTextureType::eTextureCube;
				break;
			}
			return textureParam;
		}

		TextureParam ExtractStorageTextureParam(spirv_cross::Resource const& image
			, spirv_cross::Compiler& compiler)
		{
			TextureParam textureParam{};
			ExtractBaseShaderParamInfo(textureParam, image, compiler);
			const spirv_cross::SPIRType& type = compiler.get_type(image.base_type_id);
			switch (type.image.dim)
			{
			case spv::Dim::Dim1D:
				textureParam.type =
					EShaderTextureType::eRWTexture1D;
				break;
			case spv::Dim::Dim2D:
				textureParam.type =
					EShaderTextureType::eRWTexture2D;
				break;
			case spv::Dim::Dim3D:
				textureParam.type =
					EShaderTextureType::eRWTexture3D;
				break;
			case spv::Dim::DimBuffer:
				textureParam.type =
					EShaderTextureType::eRWTexelBuffer;
				break;
			case spv::Dim::DimCube:
				CA_LOG_ERR("Strange!!!, No RWTextureCube");
				break;
			}
			return textureParam;
		}

#pragma endregion

#pragma region Buffer Params

		void ExtractBufferParamShared(BufferParam& bufferParam, spirv_cross::Resource const& buffer
			, spirv_cross::Compiler& compiler)
		{
			ExtractBaseShaderParamInfo(bufferParam, buffer, compiler);
			const spirv_cross::SPIRType& type = compiler.get_type(buffer.base_type_id);
			bufferParam.blockSize = compiler.get_declared_struct_size(type);
		}

		BufferParam ExtractStorageBufferParams(spirv_cross::Resource const& buffer
			, spirv_cross::Compiler& compiler)
		{
			BufferParam bufferParam{};
			ExtractBufferParamShared(bufferParam, buffer, compiler);
			spirv_cross::Bitset buffer_flags = compiler.get_buffer_block_flags(buffer.id);
			if (buffer_flags.get(spv::DecorationNonWritable))
			{
				bufferParam.type = EShaderBufferType::eStructuredBuffer;
			}
			else
			{
				bufferParam.type = EShaderBufferType::eRWStructuredBuffer;
			}
			return bufferParam;
		}

		BufferParam ExtractUniformBufferParams(spirv_cross::Resource const& buffer
			, spirv_cross::Compiler& compiler)
		{
			BufferParam bufferParam{};
			ExtractBufferParamShared(bufferParam, buffer, compiler);
			bufferParam.type = EShaderBufferType::eConstantBuffer;
			return bufferParam;
		}
#pragma endregion

		virtual ShaderParams ExtractShaderParams(std::vector<uint32_t> const& spirv_source) override
		{
			ShaderParams params{};
			spirv_cross::Compiler compiler(spirv_source);
			auto active_variables = compiler.get_active_interface_variables();
			spirv_cross::ShaderResources resources = compiler.get_shader_resources(active_variables);
			for (const spirv_cross::Resource& image : resources.separate_images)
			{
				//Texture2D TexelBuffer
				params.textures.push_back(ExtractTextureParam(image, compiler));
			}
			for (const spirv_cross::Resource& image : resources.storage_images)
			{
				//RWTexture2D RWTexelBuffer
				params.textures.push_back(ExtractStorageTextureParam(image, compiler));
			}
			for (const spirv_cross::Resource& image : resources.subpass_inputs)
			{
				//RWTexture2D RWTexelBuffer
				TextureParam textureParam{};
				ExtractBaseShaderParamInfo(textureParam, image, compiler);
				textureParam.type = EShaderTextureType::eSubpassInput;
				textureParam.subpassInputAttachmentID = compiler.get_decoration(image.id, spv::DecorationInputAttachmentIndex);
				params.textures.push_back(textureParam);
			}
			for (const spirv_cross::Resource& buffer : resources.uniform_buffers)
			{
				//UniformBuffer/ConstantBuffer
				params.buffers.push_back(ExtractUniformBufferParams(buffer, compiler));
			}
			for (const spirv_cross::Resource& buffer : resources.storage_buffers)
			{
				//(RW) StructuredBuffer
				params.buffers.push_back(ExtractStorageBufferParams(buffer, compiler));
			}
			for (const spirv_cross::Resource& sampler : resources.separate_samplers)
			{
				//Sampler
				SamplerParam samplerParam{};
				ExtractBaseShaderParamInfo(samplerParam, sampler, compiler);
				samplerParam.type = EShaderSamplerType::eSampler;
				params.samplers.push_back(samplerParam);
			}
			return params;
		}
	private:
		shaderc::CompileOptions m_Options;
		shaderc::Compiler		m_Compiler;
	};

	CA_LIBRARY_INSTANCE_LOADING_FUNCTIONS(IShaderCompiler, Compiler_Impl)
}

