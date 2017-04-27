/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_StaticCocoa_H
#define traktor_ui_StaticCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IStatic.h"

namespace traktor
{
	namespace ui
	{

class StaticCocoa : public WidgetCocoaImpl< IStatic, NSTextField >
{
public:
	StaticCocoa(EventSubject* owner);
	
	// IStatic implementation

	virtual bool create(IWidget* parent, const std::wstring& text) T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_StaticCocoa_H
