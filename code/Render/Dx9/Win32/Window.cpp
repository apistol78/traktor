#include <algorithm>
#include "Render/Dx9/Win32/Window.h"

namespace traktor
{
	namespace render
	{
		namespace
		{

const TCHAR* c_className = _T("TraktorRenderSystem");
const DWORD c_classIconResource = 10000;

		}

T_IMPLEMENT_RTTI_CLASS(L"traktor.render.Window", Window, Object)

Window::Window()
:	m_hWnd(0)
,	m_fullScreen(false)
{
}

Window::~Window()
{
	if (m_hWnd)
	{
		DestroyWindow(m_hWnd);
		m_hWnd = 0;
	}
}

bool Window::create(const wchar_t* title)
{
	T_ASSERT (!m_hWnd);

	WNDCLASS wc;
	std::memset(&wc, 0, sizeof(wc));
	wc.style = CS_HREDRAW | CS_VREDRAW;
	wc.cbClsExtra = 0;
	wc.cbWndExtra = sizeof(this);
	wc.lpfnWndProc = (WNDPROC)wndProc;
	wc.hInstance = static_cast< HINSTANCE >(GetModuleHandle(NULL));
	wc.hIcon = LoadIcon(wc.hInstance, MAKEINTRESOURCE(c_classIconResource));
	wc.hCursor = LoadCursor(NULL, IDC_ARROW);
	wc.lpszClassName = c_className;
	RegisterClass(&wc);

	m_hWnd = CreateWindow(
		c_className,
		title,
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

void Window::setWindowedStyle()
{
	if (m_fullScreen)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WS_OVERLAPPEDWINDOW);
		SetWindowPos(m_hWnd, HWND_NOTOPMOST, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		m_fullScreen = false;
	}
}

void Window::setFullScreenStyle()
{
	if (!m_fullScreen)
	{
		SetWindowLong(m_hWnd, GWL_STYLE, WS_POPUPWINDOW);
		SetWindowPos(m_hWnd, HWND_TOP, 0, 0, 0, 0, SWP_FRAMECHANGED | SWP_NOMOVE | SWP_NOSIZE);
		m_fullScreen = true;
	}
}

void Window::setClientSize(int32_t width, int32_t height)
{
	RECT rcWindow, rcClient;
	GetWindowRect(m_hWnd, &rcWindow);
	GetClientRect(m_hWnd, &rcClient);

	int32_t windowWidth = rcWindow.right - rcWindow.left;
	int32_t windowHeight = rcWindow.bottom - rcWindow.top;

	int32_t realClientWidth = rcClient.right - rcClient.left;
	int32_t realClientHeight = rcClient.bottom - rcClient.top;

	windowWidth = (windowWidth - realClientWidth) + width;
	windowHeight = (windowHeight - realClientHeight) + height;

	SetWindowPos(m_hWnd, NULL, 0, 0, windowWidth, windowHeight, SWP_NOZORDER | SWP_NOMOVE);
}

void Window::show()
{
	ShowWindow(m_hWnd, SW_SHOWNORMAL);
}

Window::operator HWND () const
{
	return m_hWnd;
}

void Window::addListener(IWindowListener* listener)
{
	m_listeners.push_back(listener);
}

void Window::removeListener(IWindowListener* listener)
{
	std::vector< IWindowListener* >::iterator i = std::find(m_listeners.begin(), m_listeners.end(), listener);
	m_listeners.erase(i);
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
		window = reinterpret_cast< Window* >(GetWindowLongPtr(hWnd, 0));
		if (window)
		{
			switch (message)
			{
			case WM_CREATE:
				break;

			case WM_CLOSE:
			case WM_SIZE:
				for (std::vector< IWindowListener* >::iterator i = window->m_listeners.begin(); i != window->m_listeners.end(); ++i)
					(*i)->windowListenerEvent(window, message, wParam, lParam);
				result = TRUE;
				break;

			case WM_ERASEBKGND:
				result = TRUE;
				break;

			case WM_SETCURSOR:
				if (window->m_fullScreen)
					SetCursor(NULL);
				else
					SetCursor(LoadCursor(NULL, IDC_ARROW));
				result = TRUE;
				break;
			
			default:
				result = DefWindowProc(hWnd, message, wParam, lParam);
				break;
			}
		}
		else
			result = DefWindowProc(hWnd, message, wParam, lParam);
	}

	return result;
}

	}
}
