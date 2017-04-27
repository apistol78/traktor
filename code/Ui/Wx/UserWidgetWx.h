/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_UserWidgetWx_H
#define traktor_ui_UserWidgetWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IUserWidget.h"

namespace traktor
{
	namespace ui
	{

class UserWidgetWx : public WidgetWxImpl< IUserWidget, wxWindow >
{
public:
	UserWidgetWx(EventSubject* owner);

	virtual bool create(IWidget* parent, int style);
};

	}
}

#endif	// traktor_ui_UserWidgetWx_H
