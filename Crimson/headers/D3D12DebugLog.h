#pragma once
#include <headers/D3D12Header.h>
#include <iostream>
#include <comdef.h>
namespace Crimson
{
	namespace D3D12Debug
	{
#define CHECK_DXRESULT( hresult , _error_string ) {D3D12Debug::CheckDXResultWithLine(hresult, _error_string, __LINE__, __FILE__);} 

		static inline void CheckDXResultWithLine(HRESULT result, std::string const& error_string, int line, std::string const& file)
		{
			if (FAILED(result))
			{
#ifdef _WIN32
				HANDLE hConsole = GetStdHandle(STD_OUTPUT_HANDLE);
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_INTENSITY);
#endif
				std::string out = "D3D12 Runtime Error:\n" + error_string + "\nLine: " + std::to_string(line) + "\nFile: " + file + "\n";
				std::cerr << out << std::endl;
				_com_error err(result);
				LPCTSTR errMsg = err.ErrorMessage();
				std::cerr << errMsg << std::endl;
#ifdef _WIN32
				SetConsoleTextAttribute(hConsole, FOREGROUND_RED | FOREGROUND_BLUE | FOREGROUND_GREEN);
#endif
			}
		}
	}
}