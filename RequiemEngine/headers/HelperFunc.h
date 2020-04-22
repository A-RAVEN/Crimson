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

#define MAKE_ENUM_PAIR( ENUM ) (MakeEnumFunc( #ENUM , uint32_t( ENUM ) ))
static std::pair<std::string, uint32_t> MakeEnumFunc(std::string name, uint32_t enum_num)
{
	auto find = name.find_first_of("::");
	if (find != std::string::npos)
	{
		name = name.substr(find + 2);
	}
	return std::make_pair(name, enum_num);
}