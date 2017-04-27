/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_ToolFormWx_H
#define traktor_ui_ToolFormWx_H

#include <wx/wx.h>
#include <wx/minifram.h>
#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IToolForm.h"

namespace traktor
{
	namespace ui
	{

class ToolFormWx : public WidgetWxImpl< IToolForm, wxMiniFrame >
{
public:
	ToolFormWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text, int width, int height, int style);

	virtual void center();

	virtual void setOutline(const Point* p, int np);

private:
	void onClose(wxCloseEvent& event);
};

	}
}

#endif	// traktor_ui_ToolFormWx_H
