#include <X11/Xatom.h>
#include "Ui/X11/ToolFormX11.h"

namespace traktor
{
	namespace ui
	{

ToolFormX11::ToolFormX11(EventSubject* owner, Display* display, int32_t screen)
:	WidgetX11Impl< IToolForm >(owner, display, screen)
{
}

bool ToolFormX11::create(IWidget* parent, const std::wstring& text, int width, int height, int style)
{
	const int32_t c_minWidth = 16;
	const int32_t c_minHeight = 16;

	width = std::max< int32_t >(width, c_minWidth);
	height = std::max< int32_t >(height, c_minHeight);

	Drawable window = XCreateSimpleWindow(
		m_display,
		DefaultRootWindow(m_display),
        0,
		0,
		width,
		height,
		0,
		WhitePixel(m_display, m_screen),
		WhitePixel(m_display, m_screen)
	);

    Atom type = XInternAtom(m_display,"_NET_WM_WINDOW_TYPE", False);
    Atom value = XInternAtom(m_display,"_NET_WM_WINDOW_TYPE_SPLASH", False);
    XChangeProperty(m_display, window, type, XA_ATOM, 32, PropModeReplace, reinterpret_cast<unsigned char*>(&value), 1);

	return WidgetX11Impl< IToolForm >::create(parent, window, width, height, false);
}

void ToolFormX11::destroy()
{
	WidgetX11Impl< IToolForm >::destroy();
}

void ToolFormX11::center()
{
}

	}
}

