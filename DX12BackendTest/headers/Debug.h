#pragma once
#include <assert.h>
#include <string>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#define RE_ASSERT( _condition , _log ) {if(!(_condition)){LogError(_log, __LINE__, __FILE__);}}
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