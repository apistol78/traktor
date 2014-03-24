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
	virtual bool windowListenerEvent(class Window* window, UINT message, WPARAM wParam, LPARAM lParam, LRESULT& outResult) = 0;
};

class Window : public Object
{
public:
	Window();

	virtual ~Window();

	bool create();

	void setTitle(const wchar_t* title);

	void setWindowedStyle(int32_t width, int32_t height);

	void setFullScreenStyle(int32_t width, int32_t height);

	void hide();

	operator HWND () const;

	void addListener(IWindowListener* listener);

	void removeListener(IWindowListener* listener);

	bool haveWindowedStyle() const { return !m_fullScreen; }

	bool haveFullScreenStyle() const { return m_fullScreen; }

private:
	HWND m_hWnd;
	bool m_fullScreen;
	std::set< IWindowListener* > m_listeners;
	POINT m_windowPosition;

	static LRESULT CALLBACK wndProc(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);
};

	}
}

#endif	// traktor_render_Window_H
