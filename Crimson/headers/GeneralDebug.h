#pragma once
#include <assert.h>
#include <string>
#include <iostream>

namespace Crimson
{
#define CRIM_ASSERT( _condition , _log ) {if(!(_condition)){LogError(_log, __LINE__, __FILE__);}}
#define CRIM_ASSERT_AND_RETURN_VOID( _condition , _log ) {if(!(_condition)){LogError(_log, __LINE__, __FILE__); return;}}
#define LOG_ERR(_log) {LogError(_log, __LINE__, __FILE__);}

	static inline void LogError(std::string const& log, int line, std::string const& file)
	{
#ifdef _WIN32
		HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
		std::string out = "\n" + log + "\nLine: " + std::to_string(line) + "\nFile: " + file + "\n";
		std::cerr << out << std::endl;
#ifdef _WIN32
		SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
	}
}