#pragma once
#include <headers/D3D12Header.h>
#include <iostream>

namespace Crimson
{
	namespace D3D12Debug
	{
		static inline void CheckResult(HRESULT result, std::string const& error_string)
		{
			if (FAILED(result))
			{
#ifdef _WIN32
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
				std::cerr << "D3D12 Runtime Error:\n" << error_string << std::endl;
#ifdef _WIN32
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
			}
		}
	}
}