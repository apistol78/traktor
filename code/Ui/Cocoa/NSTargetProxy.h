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

struct ITargetProxyCallback
{
	virtual void targetProxy_Action(id controlId) = 0;

	virtual void targetProxy_doubleAction(id controlId) = 0;
};

template < typename TargetType >
class TargetProxyCallbackImpl : public ITargetProxyCallback
{
public:
	typedef void (TargetType::*method_t)(id controlId);

	TargetProxyCallbackImpl(TargetType* this_, method_t action, method_t doubleAction)
	:	m_this(this_)
	,	m_action(action)
	,	m_doubleAction(doubleAction)
	{
	}

	virtual void targetProxy_Action(id controlId)
	{
		if (m_action)
			(m_this->*m_action)(controlId);
	}

	virtual void targetProxy_doubleAction(id controlId)
	{
		if (m_doubleAction)
			(m_this->*m_doubleAction)(controlId);
	}

private:
	TargetType* m_this;
	method_t m_action;
	method_t m_doubleAction;
};

	}
}

@interface NSTargetProxy : NSObject
{
	traktor::ui::ITargetProxyCallback* m_callback;
}

- (id) init;

- (void) setCallback: (traktor::ui::ITargetProxyCallback*)callback;

- (void) dispatchActionCallback: (id)controlId;

- (void) dispatchDoubleActionCallback: (id)controlId;

@end

