#ifndef traktor_ui_ContainerCocoa_H
#define traktor_ui_ContainerCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSCustomControl.h"
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
	
	// IWidget
	
	virtual void destroy();

	// IContainer

	virtual bool create(IWidget* parent, int style);
	
	// INSControlEventsCallback
	
	virtual bool event_drawRect(const NSRect& rect);
	
	virtual bool event_viewDidEndLiveResize();
	
	virtual bool event_mouseDown(NSEvent* theEvent, int button);
	
	virtual bool event_mouseUp(NSEvent* theEvent, int button);
		
	virtual bool event_mouseMoved(NSEvent* theEvent, int button);

	virtual bool event_keyDown(NSEvent* theEvent);
	
	virtual bool event_keyUp(NSEvent* theEvent);
};

	}
}

#endif	// traktor_ui_ContainerCocoa_H
