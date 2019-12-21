#pragma once
#include <IWindow.h>
#include <string>
#include <set>

using namespace Crimson;

class Win32Class
{
public:
	static Win32Class* Get();
	void InitWindowStyle(std::wstring const& style_type);
private:
	Win32Class();
	~Win32Class();
	std::set<std::wstring> m_StyleSet;
};

class Win32Window : public IWindow
{
#ifdef _WIN32
public:
	Win32Window();
	void InitWindow(std::wstring const& window_name, std::wstring const& wnd_style, uint32_t width, uint32_t height);
	virtual HINSTANCE	GetWin32Instance() override;
	virtual HWND		GetWin32Handle() override;
	void CloseWindow();
	void OnWindowResize(uint32_t new_width, uint32_t new_height);
	bool IsWindowRunning() const { return !b_Closing; }
	void UpdateWindow();
private:
	uint32_t	m_Width;
	uint32_t	m_Height;
	HINSTANCE	m_Win32Instance;
	HWND		m_Win32Window;
	bool		b_Closing;
#endif
};