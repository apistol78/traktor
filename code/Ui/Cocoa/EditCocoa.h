/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_EditCocoa_H
#define traktor_ui_EditCocoa_H

#include "Ui/Cocoa/WidgetCocoaImpl.h"
#include "Ui/Itf/IEdit.h"

namespace traktor
{
	namespace ui
	{

class EditCocoa : public WidgetCocoaImpl< IEdit, NSTextField >
{
public:
	EditCocoa(EventSubject* owner);
	
	// IEdit

	virtual bool create(IWidget* parent, const std::wstring& text, int style) T_OVERRIDE T_FINAL;

	virtual void setSelection(int from, int to) T_OVERRIDE T_FINAL;

	virtual void getSelection(int& outFrom, int& outTo) const T_OVERRIDE T_FINAL;

	virtual void selectAll() T_OVERRIDE T_FINAL;
	
	virtual void setBorderColor(const Color4ub& borderColor) T_OVERRIDE T_FINAL;

	virtual void setText(const std::wstring& text) T_OVERRIDE T_FINAL;

	virtual std::wstring getText() const T_OVERRIDE T_FINAL;
};

	}
}

#endif	// traktor_ui_EditCocoa_H
