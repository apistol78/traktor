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

	XSetWindowAttributes attributes = {};
	attributes.backing_store = WhenMapped;

	Window window = XCreateWindow(
		m_context->getDisplay(),
		parentData->window,
		0,
		0,
		64,
		64,
		0,
		0,
		InputOutput,
		CopyFromParent,
		CWBackingStore,
		&attributes
	);

	return WidgetX11Impl< IUserWidget >::create(
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

