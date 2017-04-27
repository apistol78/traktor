/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_MenuBarCocoa_H
#define traktor_ui_MenuBarCocoa_H

#import <Cocoa/Cocoa.h>
#import "Ui/Cocoa/NSTargetProxy.h"

#include "Ui/Itf/IMenuBar.h"

namespace traktor
{
	namespace ui
	{

class EventSubject;

class MenuBarCocoa
:	public IMenuBar
,	public ITargetProxyCallback
{
public:
	MenuBarCocoa(EventSubject* owner);

	// IMenuBar

	virtual bool create(IForm* form) T_OVERRIDE T_FINAL;

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual void add(MenuItem* item) T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;

private:
	EventSubject* m_owner;
	NSMenu* m_menu;
};

	}
}

#endif	// traktor_ui_MenuBarCocoa_H
