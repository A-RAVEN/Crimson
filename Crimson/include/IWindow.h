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
		virtual void UpdateWindow() = 0;
		IWindow() : m_Name(L"") {}
#ifdef _WIN32
		virtual HINSTANCE	GetWin32Instance() = 0;
		virtual HWND		GetWin32Handle() = 0;
#endif
		virtual uint32_t GetClientWidth() = 0;
		virtual uint32_t GetClientHeight() = 0;
		std::wstring const& GetName() { return m_Name; }
	protected:
		std::wstring m_Name;
	};
}