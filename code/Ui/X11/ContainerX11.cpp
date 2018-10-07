#include "Ui/X11/ContainerX11.h"

namespace traktor
{
	namespace ui
	{

ContainerX11::ContainerX11(EventSubject* owner, Display* display, int32_t screen, XIM xim)
:	WidgetX11Impl< IContainer >(owner, display, screen, xim)
{
}

bool ContainerX11::create(IWidget* parent, int style)
{
	Window parentWindow = (Window)parent->getInternalHandle();

	const int32_t c_defaultWidth = 64;
	const int32_t c_defaultHeight = 64;

	Window window = XCreateWindow(
		m_display,
		parentWindow,
		0,
		0,
		c_defaultWidth,
		c_defaultHeight,
		0,
		0,
		InputOutput,
		CopyFromParent,
		0,
		nullptr
	);

	return WidgetX11Impl< IContainer >::create(parent, style, window, Rect(0, 0, c_defaultWidth, c_defaultHeight), true);
}

	}
}

