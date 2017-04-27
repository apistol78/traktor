/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CheckBoxCocoa_H
#define traktor_ui_CheckBoxCocoa_H

#import "Ui/Cocoa/NSTargetProxy.h"

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxCocoa
:	public WidgetCocoaImpl< ICheckBox, NSButton >
,	public ITargetProxyCallback
{
public:
	CheckBoxCocoa(EventSubject* owner);
	
	// ICheckBox

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked) T_OVERRIDE T_FINAL;

	virtual void setChecked(bool checked) T_OVERRIDE T_FINAL;

	virtual bool isChecked() const T_OVERRIDE T_FINAL;
	
	// ITargetProxyCallback
	
	virtual void targetProxy_Action(void* controlId) T_OVERRIDE T_FINAL;
	
	virtual void targetProxy_doubleAction(void* controlId) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_CheckBoxCocoa_H
