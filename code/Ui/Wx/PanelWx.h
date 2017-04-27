/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_ui_PanelWx_H
#define traktor_ui_PanelWx_H

#include "Ui/Wx/WidgetWxImpl.h"
#include "Ui/Itf/IPanel.h"

namespace traktor
{
	namespace ui
	{

class PanelWx : public WidgetWxImpl< IPanel, wxPanel >
{
public:
	PanelWx(EventSubject* owner);

	virtual bool create(IWidget* parent, const std::wstring& text);

	virtual void update(const Rect* rc, bool immediate);

	virtual Rect getInnerRect() const;

	virtual Size getMinimumSize() const;
	
	virtual Size getPreferedSize() const;
	
	virtual Size getMaximumSize() const;

private:
	wxStaticBox* m_staticBox;
};

	}
}

#endif	// traktor_ui_PanelWx_H
