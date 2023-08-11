#pragma once
#include <assert.h>
#include <string>
#include <iostream>
#ifdef _WIN32
#include <Windows.h>
#endif // _WIN32


#define CA_ASSERT( _condition , _log ) {if(!(_condition)){CALogError(_log, __LINE__, __FILE__);}}
#define CA_LOG_ERR(_log) {CALogError(_log, __LINE__, __FILE__);}
#define CA_CLASS_NAME(_class) (typeid(_class).name())

static inline void CALogError(std::string const& log, int line, std::string const& file)
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