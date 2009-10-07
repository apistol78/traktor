
#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INSWindowEventsCallback
{
	virtual void event_windowDidMove() = 0;
	
	virtual void event_windowDidResize() = 0;
};

	}
}

@interface NSWindowDelegateProxy : NSObject
{
	traktor::ui::INSWindowEventsCallback* m_eventsCallback;
}

- (void)setCallback: (traktor::ui::INSWindowEventsCallback*)eventsCallback;

- (void)windowDidMove: (NSNotification*)notification;

- (void)windowDidResize: (NSNotification*)notification;

@end
