#ifndef traktor_ui_UserWidgetCocoa_H
#define traktor_ui_UserWidgetCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Cocoa/NSCustomControl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetCocoa
:	public WidgetCocoaImpl< IUserWidget, NSCustomControl >
,	public INSControlEventsCallback
{
public:
	UserWidgetCocoa(EventSubject* owner);
	
	// IWidget
	
	virtual void destroy();

	// IUserWidget

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

#endif	// traktor_ui_UserWidgetCocoa_H
