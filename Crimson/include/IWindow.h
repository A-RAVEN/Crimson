#pragma once
#include <string>
#ifdef _WIN32
#include <Windows.h>
#endif

namespace Crimson
{
	class IWindow
	{
	public:
		IWindow() : m_Name(L"") {}
#ifdef _WIN32
		virtual HINSTANCE	GetWin32Instance() = 0;
		virtual HWND		GetWin32Handle() = 0;
#endif
		std::wstring const& GetName() { return m_Name; }
	protected:
		std::wstring m_Name;
	};
}