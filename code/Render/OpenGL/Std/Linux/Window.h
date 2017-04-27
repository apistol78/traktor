/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
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

	void setFullScreenStyle();

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

	// X11 protocol atoms.
	Atom m_atomWmBypassCompositor;
	Atom m_atomWmState;
	Atom m_atomWmStateFullscreen;
	Atom m_atomWmStateMaximizedVert;
	Atom m_atomWmStateMaximizedHorz;
	Atom m_atomWmStateAbove;
	Atom m_atomWmDeleteWindow;

	int32_t m_screen;
	int32_t m_width;
	int32_t m_height;
	bool m_fullScreen;
	bool m_active;
	bool m_cursorShow;
	bool m_cursorShown;

	void setWmProperty(Atom property, int32_t value);

	void setWmProperty(Atom property1, Atom property2, int32_t value);
};

	}
}

#endif  // traktor_render_Window_H
