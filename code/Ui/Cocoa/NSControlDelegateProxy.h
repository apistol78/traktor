
#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INSControlEventsCallback
{
	virtual bool event_drawRect(const NSRect& rect) = 0;

	virtual bool event_viewDidEndLiveResize() = 0;
	
	virtual bool event_mouseDown(NSEvent* theEvent) = 0;
	
	virtual bool event_mouseUp(NSEvent* theEvent) = 0;
		
	virtual bool event_mouseMoved(NSEvent* theEvent) = 0;
};

	}
}

@interface NSControlDelegateProxy : NSObject
{
	traktor::ui::INSControlEventsCallback* m_eventsCallback;
}

- (void)setCallback: (traktor::ui::INSControlEventsCallback*)eventsCallback;

- (void)drawRect: (NSRect)rect;

- (void)viewDidEndLiveResize;

- (void) mouseDown: (NSEvent*)theEvent;

- (void) mouseUp: (NSEvent*)theEvent;

- (void) mouseMoved: (NSEvent*)theEvent;

@end
