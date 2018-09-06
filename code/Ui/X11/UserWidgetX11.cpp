#include "Ui/X11/UserWidgetX11.h"

namespace traktor
{
	namespace ui
	{

UserWidgetX11::UserWidgetX11(EventSubject* owner, Display* display, int32_t screen)
:	WidgetX11Impl< IUserWidget >(owner, display, screen)
{
}

bool UserWidgetX11::create(IWidget* parent, int style)
{
	Drawable parentWindow = (Drawable)parent->getInternalHandle();

	const int32_t c_defaultWidth = 64;
	const int32_t c_defaultHeight = 64;

	Drawable window = XCreateSimpleWindow(
		m_display,
		parentWindow,
        0,
		0,
		c_defaultWidth,
		c_defaultHeight,
		0,
		WhitePixel(m_display, m_screen),
		WhitePixel(m_display, m_screen)
	);

	XFlush(m_display);

	return WidgetX11Impl< IUserWidget >::create(parent, window, c_defaultWidth, c_defaultHeight, true);
}

	}
}

