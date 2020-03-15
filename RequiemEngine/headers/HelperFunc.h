#pragma once
#include <string>
#include <fstream>

static std::string LoadStringFile(std::string file_source)
{
	std::ifstream file_src(file_source);
	std::string result;
	if (file_src.is_open())
	{
		std::string line;
		while (std::getline(file_src, line))
		{
			result += line + "\n";
		}
	}
	return result;
}