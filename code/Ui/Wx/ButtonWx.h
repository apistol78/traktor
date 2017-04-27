/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ButtonWx_H
#define traktor_ui_ButtonWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IButton.h"

namespace traktor
{
	namespace ui
	{

class ButtonWx : public WidgetWxImpl< IButton, wxControl >
{
public:
	ButtonWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int style);

	virtual void setState(bool state);

	virtual bool getState() const;

private:
	void onClick(wxCommandEvent& event);
};

	}
}

#endif	// traktor_ui_ButtonWx_H
