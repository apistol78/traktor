#ifndef traktor_ui_NSNotificationProxy_H
#define traktor_ui_NSNotificationProxy_H

#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct INotificationProxyCallback
{
	virtual void notificationProxy_recv(::NSNotification* notification) = 0;
};

template < typename TargetType >
class NotificationProxyCallbackImpl : public INotificationProxyCallback
{
public:
	typedef void (TargetType::*method_t)(::NSNotification* notification);

	NotificationProxyCallbackImpl(TargetType* this_, method_t recv)
	:	m_this(this_)
	,	m_recv(recv)
	{
	}
	
	virtual void notificationProxy_recv(::NSNotification* notification)
	{
		(m_this->*m_recv)(notification);
	}
	
private:
	TargetType* m_this;
	method_t m_recv;
};

	}
}

@interface NSNotificationProxy : NSObject
{
	traktor::ui::INotificationProxyCallback* m_callback;
}

- (id) init;

- (void) setCallback: (traktor::ui::INotificationProxyCallback*)callback;

- (void) dispatchNotificationCallback: (NSNotification*)notification;

@end

#endif	// traktor_ui_NSNotificationProxy_H
