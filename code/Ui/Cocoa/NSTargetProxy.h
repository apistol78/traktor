/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_NSTargetProxy_H
#define traktor_ui_NSTargetProxy_H

#import <Cocoa/Cocoa.h>

namespace traktor
{
	namespace ui
	{
	
struct ITargetProxyCallback
{
	virtual void targetProxy_Action(void* controlId) = 0;
	
	virtual void targetProxy_doubleAction(void* controlId) = 0;
};

template < typename TargetType >
class TargetProxyCallbackImpl : public ITargetProxyCallback
{
public:
	typedef void (TargetType::*method_t)(void* controlId);

	TargetProxyCallbackImpl(TargetType* this_, method_t action, method_t doubleAction)
	:	m_this(this_)
	,	m_action(action)
	,	m_doubleAction(doubleAction)
	{
	}
	
	virtual void targetProxy_Action(void* controlId)
	{
		if (m_action)
			(m_this->*m_action)(controlId);
	}
	
	virtual void targetProxy_doubleAction(void* controlId)
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

#endif	// traktor_ui_NSTargetProxy_H
