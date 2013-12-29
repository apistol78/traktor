#ifndef traktor_render_Window_H
#define traktor_render_Window_H

#include <X11/Xlib.h>
#include <X11/Xatom.h>
#include <X11/Xutil.h>
#include <X11/cursorfont.h>
#include <X11/extensions/Xrandr.h>
#include "Core/Object.h"
#include "Render/Types.h"

namespace traktor
{
	namespace render
	{

class Window : public Object
{
public:
	Window(::Display* display);

	virtual ~Window();

	bool create(int32_t width, int32_t height);

	void setTitle(const wchar_t* title);

	void setFullScreenStyle(int32_t width, int32_t height);

	void setWindowedStyle(int32_t width, int32_t height);

	void showCursor();

	void hideCursor();

	void show();

	void center();

	bool update(RenderEvent& outEvent);

	int32_t getWidth() const { return m_width; }

	int32_t getHeight() const { return m_height; }

	bool isFullScreen() const { return m_fullScreen; }

	bool isActive() const { return m_active; }

	::Display* getDisplay() const { return m_display; }

	::Window getWindow() const { return m_window; }

private:
	::Display* m_display;
	::Window m_window;
	int32_t m_screen;
	int32_t m_width;
	int32_t m_height;
	bool m_fullScreen;
	bool m_active;
	XRRScreenConfiguration* m_originalConfig;
	int32_t m_originalSizeIndex;
	int32_t m_originalRate;
	Rotation m_originalRotation;
};

	}
}

#endif  // traktor_render_Window_H
