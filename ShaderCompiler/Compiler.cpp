#ifdef _WIN32
#include <Windows.h>
#endif
#define NV_EXTENSIONS
#include <include/Compiler.h>
#include <FileIncluder.h>
#include <shaderc/shaderc.hpp>
#include <iostream>


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

	class Compiler_Impl : public IShaderCompiler
	{
	public:
		Compiler_Impl()
		{
			m_Options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_2);
			m_Options.SetTargetSpirv(shaderc_spirv_version_1_3);
			m_Options.SetIncluder(std::unique_ptr<shaderc::CompileOptions::IncluderInterface>(&m_Includer));
		}
		~Compiler_Impl()
		{

		}
		virtual void AddInlcudePath(std::string const& path) override
		{
			if (!m_Includer.AddIncludePath(path))
			{
#ifdef _WIN32
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
				std::cerr << "Shader Compiler Add Path:\n" << path << "\nFailed!" << std::endl;
#ifdef _WIN32
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
			}
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
		virtual std::string PreprocessGLSLShader(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type) override
		{
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
		virtual std::vector<uint32_t> CompileGLSLShaderSource(std::string const& file_name, std::string const& shader_src, ECompileShaderType shader_type, bool optimize = true)
		{
			if (optimize)
			{
				m_Options.SetOptimizationLevel(shaderc_optimization_level_performance);
			}

			shaderc::SpvCompilationResult result =
				m_Compiler.CompileGlslToSpv(shader_src, SHADER_KIND_TABLE[static_cast<uint32_t>(shader_type)], file_name.c_str(), m_Options);

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
	private:
		shaderc::CompileOptions m_Options;
		FileIncluder			m_Includer;
		shaderc::Compiler		m_Compiler;
	};

	IShaderCompiler* IShaderCompiler::GetCompiler()
	{
		if (m_Singleton == nullptr)
		{
			m_Singleton = new Compiler_Impl();
		}
		return m_Singleton;
	}

	void IShaderCompiler::DisposeCompiler()
	{
		if (m_Singleton != nullptr)
		{
			delete m_Singleton;
			m_Singleton = nullptr;
		}
	}


}