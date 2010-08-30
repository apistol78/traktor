
#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INSWindowEventsCallback
{
	virtual void event_windowDidMove() = 0;
	
	virtual void event_windowDidResize() = 0;
	
	virtual bool event_windowShouldClose() = 0;
};

	}
}

@interface NSWindowDelegateProxy : NSObject
{
	traktor::ui::INSWindowEventsCallback* m_eventsCallback;
}

- (id) init;

- (void) setCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback;

- (void) windowDidMove: (NSNotification*)notification;

- (void) windowDidResize: (NSNotification*)notification;

- (BOOL) windowShouldClose: (id)sender;

@end
