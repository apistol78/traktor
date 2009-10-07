#ifndef traktor_ui_ContainerCocoa_H
#define traktor_ui_ContainerCocoa_H

#import "Ui/Cocoa/NSCustomControl.h"
#import "Ui/Cocoa/NSControlDelegateProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IContainer.h"

namespace traktor
{
	namespace ui
	{

class ContainerCocoa
:	public WidgetCocoaImpl< IContainer, NSCustomControl >
,	public INSControlEventsCallback
{
public:
	ContainerCocoa(EventSubject* owner);

	// IContainer

	virtual bool create(IWidget* parent, int style);
	
	// INSControlEventsCallback
	
	virtual bool event_drawRect(const NSRect& rect);
	
	virtual bool event_viewDidEndLiveResize();
	
	virtual bool event_mouseDown(NSEvent* theEvent);
	
	virtual bool event_mouseUp(NSEvent* theEvent);
		
	virtual bool event_mouseMoved(NSEvent* theEvent);
};

	}
}

#endif	// traktor_ui_ContainerCocoa_H
