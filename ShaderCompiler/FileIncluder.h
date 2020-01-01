#pragma once
#include <unordered_set>
#include <shaderc/shaderc.hpp>
#include <filesystem>

namespace ShaderCompiler
{
	class FileIncluder : public shaderc::CompileOptions::IncluderInterface {
	public:
		FileIncluder();
		~FileIncluder() override;

		// Resolves a requested source file of a given type from a requesting
		// source into a shaderc_include_result whose contents will remain valid
		// until it's released.
		shaderc_include_result* GetInclude(const char* requested_source,
			shaderc_include_type type,
			const char* requesting_source,
			size_t include_depth) override;
		// Releases an include result.
		void ReleaseInclude(shaderc_include_result* include_result) override;

		// Returns a reference to the member storing the set of included files.
		const std::unordered_set<std::string>& file_path_trace() const {
			return m_IncludeFiles;
		}

		bool AddIncludePath(std::string const& path);

		std::string FindReadableFilepath(std::string const& requested_src);
		std::string FindRelativeReadableFilepath(std::string const& requesting_src, std::string const& requested_src);
	private:
		// Used by GetInclude() to get the full filepath.
		//const shaderc_util::FileFinder& file_finder_;
		// The full path and content of a source file.
		struct FileInfo {
			const std::string full_path;
			std::string contents;
		};

		// The set of full paths of included files.
		std::unordered_set<std::string> m_IncludeFiles;

		std::vector<std::filesystem::path> m_SearchPaths;
	};

}