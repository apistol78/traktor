#ifndef traktor_ui_UserWidgetCocoa_H
#define traktor_ui_UserWidgetCocoa_H

#import "Ui/Cocoa/NSCustomControl.h"
#import "Ui/Cocoa/NSControlDelegateProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
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

	// IUserWidget implementation

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

#endif	// traktor_ui_UserWidgetCocoa_H
