#include "Ui/Cocoa/ContainerCocoa.h"
#include "Ui/Events/SizeEvent.h"
#include "Ui/EventSubject.h"

namespace traktor
{
	namespace ui
	{
	
ContainerCocoa::ContainerCocoa(EventSubject* owner)
:	WidgetCocoaImpl< IContainer, NSCustomControl >(owner)
{
}

bool ContainerCocoa::create(IWidget* parent, int style)
{
	m_control = [[NSCustomControl alloc]
		initWithFrame: NSMakeRect(0, 0, 0, 0)
	];
	
	NSControlDelegateProxy* proxy = [[NSControlDelegateProxy alloc] init];
	[proxy setCallback: this];
	
	[m_control setDelegate: proxy];
	
	NSView* contentView = (NSView*)parent->getInternalHandle();
	T_ASSERT (contentView);
	
	[contentView addSubview: m_control];

	return true;
}

bool ContainerCocoa::event_drawRect(const NSRect& rect)
{
	return false;
}

bool ContainerCocoa::event_viewDidEndLiveResize()
{
	Size sz = getRect().getSize();
	SizeEvent s(m_owner, 0, sz);
	m_owner->raiseEvent(EiSize, &s);
	return true;
}

bool ContainerCocoa::event_mouseDown(NSEvent* theEvent)
{
	return true;
}

bool ContainerCocoa::event_mouseUp(NSEvent* theEvent)
{
	return true;
}
	
bool ContainerCocoa::event_mouseMoved(NSEvent* theEvent)
{
	return true;
}
	
	}
}
