#include <algorithm>
#include "Render/OpenGL/Std/Win32/Window.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystemOpenGL");
const DWORD c_classIconResource = 10000;

		}

Window::Window()
:	m_hWnd(0)
,	m_fullScreen(false)
{
	std::memset(&m_windowPosition, 0, sizeof(m_windowPosition));
}

Window::~Window()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

bool Window::create()
{
	T_ASSERT (!m_hWnd);

	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = 0;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast< HINSTANCE >(GetModuleHandle(NULL));
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(c_classIconResource));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.hbrBackground = (HBRUSH)GetStockObject(BLACK_BRUSH);
	wc.lpszClassName = c_className;
	RegisterClass(&wc);

	m_hWnd = CreateWindow(
		c_className,
		L"",
		WS_OVERLAPPEDWINDOW,
		CW_USEDEFAULT,
		CW_USEDEFAULT,
		64,
		64,
		NULL,
		NULL,
		static_cast< HMODULE >(GetModuleHandle(NULL)),
		this
	);
	if (!m_hWnd)
		return false;

	return true;
}

void Window::setTitle(const wchar_t* title)
{
	SetWindowText(m_hWnd, title ? title : L"");
}

void Window::setWindowedStyle(int32_t width, int32_t height)
{
	if (m_fullScreen)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_hWnd, NULL, m_windowPosition.x, m_windowPosition.y, 0, 0, SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
		SetCursor(LoadCursor(NULL, IDC_ARROW));
	}

	if (m_fullScreen || !IsWindowVisible(m_hWnd))
		ShowWindow(m_hWnd, SW_SHOWNOACTIVATE);

	RECT rcWindow, rcClient;
	GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	if (realClientWidth != width || realClientHeight != height)
	{
		windowWidth = (windowWidth - realClientWidth) + width;
		windowHeight = (windowHeight - realClientHeight) + height;
		SetWindowPos(m_hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOMOVE | SWP_NOZORDER);
	}
	
	m_fullScreen = false;
}

void Window::setFullScreenStyle()
{
	if (!m_fullScreen)
	{
		RECT rcWindow;
		GetWindowRect(m_hWnd, &rcWindow);

		m_windowPosition.x = rcWindow.left;
		m_windowPosition.y = rcWindow.top;

		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUP);
		SetWindowPos(m_hWnd, NULL, 0, 0, 0, 0, SWP_NOMOVE | SWP_NOSIZE | SWP_NOZORDER | SWP_FRAMECHANGED);
	}

	ShowWindow(m_hWnd, SW_MAXIMIZE);
	m_fullScreen = true;
}

void Window::hide()
{
	ShowWindow(m_hWnd, SW_HIDE);
}

Window::operator HWND () const
{
	return m_hWnd;
}

void Window::addListener(IWindowListener* listener)
{
	m_listeners.insert(listener);
}

void Window::removeListener(IWindowListener* listener)
{
	m_listeners.erase(listener);
}

LRESULT CALLBACK Window::wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	LRESULT result = FALSE;
	Window* window = 0;

	if (message == WM_CREATE)
	{
		LPCREATESTRUCT createStruct = reinterpret_cast< LPCREATESTRUCT >(lParam);
		window = reinterpret_cast< Window* >(createStruct->lpCreateParams);
		SetWindowLongPtr(hWnd, 0, reinterpret_cast< LONG_PTR >(window));
	}
	else
	{
		bool handled = false;

		window = reinterpret_cast< Window* >(GetWindowLongPtr(hWnd, 0));
		if (window)
		{
			for (std::set< IWindowListener* >::iterator i = window->m_listeners.begin(); i != window->m_listeners.end(); ++i)
				handled |= (*i)->windowListenerEvent(window, message, wParam, lParam, result);
		}

		if (!handled)
			result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return result;
}

	}
}
