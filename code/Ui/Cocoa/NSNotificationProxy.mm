#import "Ui/Cocoa/NSNotificationProxy.h"

@implementation NSNotificationProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_callback = 0;
	return self;
}

- (void) setCallback: (proxy_callback_t*)callback
{
	m_callback = callback;
}

- (void) dispatchNotificationCallback: (NSNotification*)notification
{
	if (m_callback)
		m_callback->notificationProxy_recv(notification);
}

@end
