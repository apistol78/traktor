/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_RadioButtonWx_H
#define traktor_ui_RadioButtonWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IRadioButton.h"

namespace traktor
{
	namespace ui
	{

class RadioButtonWx : public WidgetWxImpl< IRadioButton, wxRadioButton >
{
public:
	RadioButtonWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;
};

	}
}

#endif	// traktor_ui_RadioButtonWx_H
