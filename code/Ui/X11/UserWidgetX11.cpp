#include "Ui/X11/UserWidgetX11.h"

namespace traktor
{
	namespace ui
	{

UserWidgetX11::UserWidgetX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IUserWidget >(context, owner)
{
}

bool UserWidgetX11::create(IWidget* parent, int style)
{
	WidgetData* parentData = static_cast< WidgetData* >(parent->getInternalHandle());

	const int32_t c_defaultWidth = 64;
	const int32_t c_defaultHeight = 64;

	Window window = XCreateWindow(
		m_context->getDisplay(),
		parentData->window,
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

	return WidgetX11Impl< IUserWidget >::create(parent, style, window, Rect(0, 0, c_defaultWidth, c_defaultHeight), true, false);
}

	}
}

