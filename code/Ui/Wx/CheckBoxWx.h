/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_CheckBoxWx_H
#define traktor_ui_CheckBoxWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/ICheckBox.h"

namespace traktor
{
	namespace ui
	{

class CheckBoxWx : public WidgetWxImpl< ICheckBox, wxCheckBox >
{
public:
	CheckBoxWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, bool checked);

	virtual void setChecked(bool checked);

	virtual bool isChecked() const;

private:
	void onClicked(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_CheckBoxWx_H
