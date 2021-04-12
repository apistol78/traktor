#include "Ui/X11/ContainerX11.h"

namespace traktor
{
	namespace ui
	{

ContainerX11::ContainerX11(Context* context, EventSubject* owner)
:	WidgetX11Impl< IContainer >(context, owner)
{
}

bool ContainerX11::create(IWidget* parent, int style)
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

	return WidgetX11Impl< IContainer >::create(
		parent,
		style,
		window,
		Rect(0, 0, 0, 0),
		true,
		false
	);
}

	}
}

