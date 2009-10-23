#import "Ui/Cocoa/NSNotificationProxy.h"

@implementation NSNotificationProxy

- (id) init
{
	if ((self = [super init]) != nil)
		m_callback = 0;
	return self;
}

- (void) setCallback: (traktor::ui::INotificationProxyCallback*)callback
{
	m_callback = callback;
}

- (void) dispatchNotificationCallback: (NSNotification*)notification
{
	if (m_callback)
		m_callback->notificationProxy_recv(notification);
}

@end
