#ifndef traktor_render_Window_H
#define traktor_render_Window_H

#define _WIN32_LEAN_AND_MEAN
#ifndef _WIN32_WINNT
#	define _WIN32_WINNT 0x0501
#endif
#include <windows.h>
#include <tchar.h>
#include "Core/Object.h"

namespace traktor
{
	namespace render
	{

struct IWindowListener
{
	virtual void windowListenerEvent(class Window* window, UINT message, WPARAM wParam, LPARAM lParam) = 0;
};

class Window : public Object
{
	T_RTTI_CLASS;

public:
	Window();

	virtual ~Window();

	bool create(const wchar_t* title);

	void setWindowedStyle();

	void setFullScreenStyle();

	void setClientSize(int32_t width, int32_t height);

	void show();

	operator HWND () const;

	void addListener(IWindowListener* listener);

	void removeListener(IWindowListener* listener);

private:
	HWND m_hWnd;
	bool m_fullScreen;
	std::vector< IWindowListener* > m_listeners;

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_Window_H
