/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

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

typedef traktor::ui::INotificationProxyCallback proxy_callback_t;

@interface NSNotificationProxy : NSObject
{
	proxy_callback_t* m_callback;
}

- (id) init;

- (void) setCallback: (proxy_callback_t*)callback;

- (void) dispatchNotificationCallback: (NSNotification*)notification;

@end

