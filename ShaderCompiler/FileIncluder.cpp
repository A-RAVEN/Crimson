#include "FileIncluder.h"
#include <filesystem>
#include <fstream>
#include <stdint.h>

namespace ShaderCompiler
{
	using namespace ::std::filesystem;

	shaderc_include_result* MakeErrorIncludeResult(const char* message) {
		return new shaderc_include_result{ "", 0, message, strlen(message) };
	}

	bool FileIncluder::AddIncludePath(std::string const& path)
	{
		std::filesystem::path new_path(path);
		if (is_directory(new_path))
		{
			m_SearchPaths.push_back(new_path);
			return true;
		}
		else if (is_regular_file(new_path))
		{
			m_SearchPaths.push_back(new_path.parent_path());
			return true;
		}
		return false;
	}

	std::string FileIncluder::FindReadableFilepath(std::string const& requested_src)
	{
		for (auto& dir : m_SearchPaths)
		{
			auto full_path = dir / requested_src;
			if(is_regular_file(full_path))
			{
				return absolute(full_path).string();
			}
		}
		return "";
	}

	std::string FileIncluder::FindRelativeReadableFilepath(std::string const& requesting_src, std::string const& requested_src)
	{
		path src(requested_src);
		if (is_regular_file(src))
		{
			auto parent_path = src.parent_path();
			auto find_src = parent_path / requested_src;
			if (is_regular_file(find_src))
			{
				return absolute(find_src).string();
			}
			else
			{
				return FindReadableFilepath(requested_src);
			}
		}
		return "";
	}



	FileIncluder::FileIncluder()
	{
	}

	FileIncluder::~FileIncluder()
	{
	}

	shaderc_include_result* FileIncluder::GetInclude(
		const char* requested_source, shaderc_include_type include_type,
		const char* requesting_source, size_t) {

		const std::string full_path =
			(include_type == shaderc_include_type_relative)
			? FindRelativeReadableFilepath(requesting_source,
				requested_source)
			: FindReadableFilepath(requested_source);

		if (full_path.empty())
			return MakeErrorIncludeResult("Cannot find or open include file.");

		// In principle, several threads could be resolving includes at the same
		// time.  Protect the included_files.

		// Read the file and save its full path and contents into stable addresses.
		FileIncluder::FileInfo* new_file_info = new FileIncluder::FileInfo{ full_path, {} };
		std::ifstream file_str(full_path);
		if(!file_str.is_open()){
			return MakeErrorIncludeResult("Cannot read file");
		}
		{
			std::string new_line;
			while (std::getline(file_str, new_line))
			{
				new_file_info->contents += new_line + '\n';
			}
			file_str.close();
		}

		m_IncludeFiles.insert(full_path);

		return new shaderc_include_result{
			new_file_info->full_path.data(), new_file_info->full_path.length(),
			new_file_info->contents.data(), new_file_info->contents.size(),
			new_file_info };
	}

	void FileIncluder::ReleaseInclude(shaderc_include_result* include_result) {
		FileInfo* info = static_cast<FileInfo*>(include_result->user_data);
		delete info;
		delete include_result;
	}
}