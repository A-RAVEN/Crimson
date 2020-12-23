#include <headers/Win32Window.h>

static LRESULT CALLBACK WindowsEventHandler(HWND hWnd, UINT uMsg, WPARAM wParam, LPARAM lParam)
{
	Win32Window* p_window = reinterpret_cast<Win32Window*>(GetWindowLongPtr(hWnd, GWLP_USERDATA));

	switch (uMsg)
	{
	case WM_SETFOCUS:
		printf("WM_SETFOCUS");
		p_window->SetFocus(true);
		break;

	case WM_KILLFOCUS:
		printf("WM_KILLFOCUS");
		p_window->SetFocus(false);

		break;
	case WM_CLOSE:
		printf("WM_CLOSE");
		p_window->CloseWindow();
		return 0;
	case WM_SIZE:
		//VulkanSurface£¬Image£¬ViewPort£¬ortho/perspective
		printf("WM_SIZE");
		p_window->OnWindowResize(LOWORD(lParam), HIWORD(lParam));
		return 0;
	case WM_MOUSEMOVE:
	{
		//if (true)
		//{
		//	for (auto func : window->pMousePositionFunc)
		//		func(LOWORD(lParam), HIWORD(lParam));
		//}
		//return 0;
	}
	case WM_LBUTTONDOWN: case WM_LBUTTONDBLCLK:
	case WM_RBUTTONDOWN: case WM_RBUTTONDBLCLK:
	case WM_MBUTTONDOWN: case WM_MBUTTONDBLCLK:
	case WM_XBUTTONDOWN: case WM_XBUTTONDBLCLK:
	{
		//EMouseButtom mouse_type = E_MOUSE_BUTTOM_MAX;
		//if (uMsg == WM_LBUTTONDOWN || uMsg == WM_LBUTTONDBLCLK) { mouse_type = E_LB; }
		//if (uMsg == WM_RBUTTONDOWN || uMsg == WM_RBUTTONDBLCLK) { mouse_type = E_RB; }
		//if (uMsg == WM_MBUTTONDOWN || uMsg == WM_MBUTTONDBLCLK) { mouse_type = E_MB; }
		//if (true)
		//{
		//	for (auto func : window->pMouseButtomFunc)
		//		func(mouse_type, true);
		//}
		//return 0;
	}
	case WM_LBUTTONUP:
	case WM_RBUTTONUP:
	case WM_MBUTTONUP:
	case WM_XBUTTONUP:
	{
		//EMouseButtom mouse_type = E_MOUSE_BUTTOM_MAX;
		//if (uMsg == WM_LBUTTONUP) { mouse_type = E_LB; }
		//if (uMsg == WM_RBUTTONUP) { mouse_type = E_RB; }
		//if (uMsg == WM_MBUTTONUP) { mouse_type = E_MB; }
		//if (true)
		//{
		//	for (auto func : window->pMouseButtomFunc)
		//		func(mouse_type, false);
		//}
		//return 0;
	}
	//case WM_KEYDOWN:
	//case WM_SYSKEYDOWN:
	//	if (wParam < 256)
	//	{
	//		//for (auto func : window->pKeyInputFunc)
	//		//	func(uint8_t(wParam), true);
	//	}
	//	return 0;
	//case WM_KEYUP:
	//case WM_SYSKEYUP:
	//	if (wParam < 256)
	//	{
	//		//for (auto func : window->pKeyInputFunc)
	//		//	func(uint8_t(wParam), false);
	//	}
	//	return 0;
	//case WM_CHAR:
	//	if (true)
	//	{
	//		//for (auto func : window->pCharInputFunc)
	//		//	func(char(wParam));
	//	}
	//	return 0;
	default:
		break;
	}
	return DefWindowProc(hWnd, uMsg, wParam, lParam);
}

Win32Window::Win32Window() :
	m_Width(0),
	m_Height(0),
	m_Win32Instance(NULL),
	m_Win32Window(NULL),
	b_Closing(false)
{
}

void Win32Window::InitWindow(std::wstring const& window_name, std::wstring const& wnd_style, uint32_t width, uint32_t height)
{
	m_Name = window_name;
	m_Width = width;
	m_Height = height;
	Win32Class::Get()->InitWindowStyle(wnd_style);
	m_Win32Instance = GetModuleHandle(NULL);

	DWORD ex_style = WS_EX_APPWINDOW | WS_EX_WINDOWEDGE;
	DWORD style = WS_OVERLAPPEDWINDOW;

	//create window using the 
	RECT wr = { 0, 0, LONG(width), LONG(height) };
	AdjustWindowRectEx(&wr, style, FALSE, ex_style);
	m_Win32Window = CreateWindowEx(
		ex_style,
		wnd_style.c_str(),//class name
		window_name.c_str(),//app name
		style,//window style
		CW_USEDEFAULT, CW_USEDEFAULT,//x/y coords
		wr.right - wr.left,//width
		wr.bottom - wr.top,//height
		NULL,//handle to parent
		NULL,//handle to menu
		m_Win32Instance,//hInstance
		NULL//no extra parameters
	);
	if (!m_Win32Window)
	{
		//window build failed
		std::exit(-1);
	}
	SetWindowLongPtr(m_Win32Window, GWLP_USERDATA, reinterpret_cast<LONG_PTR>(this));

	ShowWindow(m_Win32Window, SW_SHOW);
	SetForegroundWindow(m_Win32Window);
	SetFocus(m_Win32Window);
}

HINSTANCE Win32Window::GetWin32Instance()
{
	return m_Win32Instance;
}

HWND Win32Window::GetWin32Handle()
{
	return m_Win32Window;
}

void Win32Window::CloseWindow()
{
	b_Closing = true;
}

void Win32Window::OnWindowResize(uint32_t new_width, uint32_t new_height)
{
}

void Win32Window::UpdateWindow()
{
	MSG msg;
	//if (PeekMessage(&msg, m_Win32Window, 0, 0, PM_REMOVE))
	if (PeekMessageA(&msg, NULL, 0, 0, PM_REMOVE))
	{
		TranslateMessage(&msg);
		DispatchMessage(&msg);
	}
}

Win32Class* Win32Class::Get()
{
	static Win32Class static_singleton{};
	return &static_singleton;
}

void Win32Class::InitWindowStyle(std::wstring const& style_type)
{
	if (m_StyleSet.find(style_type) != m_StyleSet.end()) { return; }
	if (style_type == L"default")
	{
		WNDCLASSEX win_class = {};
		win_class.cbSize = sizeof(WNDCLASSEX);
		win_class.style = CS_HREDRAW | CS_VREDRAW;
		win_class.lpfnWndProc = WindowsEventHandler;
		win_class.cbClsExtra = 0;
		win_class.cbWndExtra = 0;
		win_class.hInstance = GetModuleHandle(NULL);
		win_class.hIcon = LoadIcon(NULL, IDI_APPLICATION);
		win_class.hCursor = LoadCursor(NULL, IDC_ARROW);
		win_class.hbrBackground = (HBRUSH)GetStockObject(WHITE_BRUSH);
		win_class.lpszMenuName = NULL;
		win_class.lpszClassName = style_type.c_str();
		win_class.hIconSm = LoadIcon(NULL, IDI_WINLOGO);
		//register windows class
		if (!RegisterClassEx(&win_class))
		{
			std::exit(-1);
		}
	}
	else if (style_type == L"fullscreen")
	{

	}
	else
	{
		return;
	}
	m_StyleSet.insert(style_type);
}

Win32Class::Win32Class()
{
}

Win32Class::~Win32Class()
{
	HINSTANCE instance = GetModuleHandle(NULL);
	for (auto& name : m_StyleSet)
	{
		UnregisterClass(name.c_str(), instance);
	}
}
