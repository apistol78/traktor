/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ButtonCocoa_H
#define traktor_ui_ButtonCocoa_H

#import "Ui/Cocoa/NSTargetProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

class ButtonCocoa
:	public WidgetCocoaImpl< IButton, NSButton >
,	public ITargetProxyCallback
{
public:
	ButtonCocoa(EventSubject* owner);
	
	// IButton

	virtual bool create(IWidget* parent, const std::wstring& text, int style) T_OVERRIDE T_FINAL;

	virtual void setState(bool state) T_OVERRIDE T_FINAL;

	virtual bool getState() const T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_ButtonCocoa_H
