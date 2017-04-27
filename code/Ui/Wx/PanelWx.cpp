/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Wx/PanelWx.h"

namespace traktor
{
	namespace ui
	{

PanelWx::PanelWx(EventSubject* owner)
:	WidgetWxImpl< IPanel, wxPanel >(owner)
{
}

bool PanelWx::create(IWidget* parent, const std::wstring& text)
{
	if (!parent)
		return false;
		
	m_window = new wxPanel();
	if (!m_window->Create(
		static_cast< wxWindow* >(parent->getInternalHandle()),
		-1
	))
	{
		m_window->Destroy();
		return false;
	}
	
	m_staticBox = new wxStaticBox();
	if (!m_staticBox->Create(
		m_window,
		-1,
		wstots(text).c_str()
	))
	{
		m_staticBox->Destroy();
		m_window->Destroy();
		return false;
	}

	if (!WidgetWxImpl< IPanel, wxPanel >::create(0))
		return false;
	
	return true;
}

void PanelWx::update(const Rect* rc, bool immediate)
{
	m_staticBox->SetSize(m_window->GetClientSize());
	WidgetWxImpl< IPanel, wxPanel >::update(rc, immediate);
}

Rect PanelWx::getInnerRect() const
{
	Rect rc = WidgetWxImpl< IPanel, wxPanel >::getInnerRect();
	rc.left += 8;
	rc.top += 16;
	rc.right -= 8;
	rc.bottom -= 8;
	return rc;
}

Size PanelWx::getMinimumSize() const
{
	return WidgetWxImpl< IPanel, wxPanel >::getMinimumSize() + Size(16, 24);
}

Size PanelWx::getPreferedSize() const
{
	return WidgetWxImpl< IPanel, wxPanel >::getPreferedSize() + Size(16, 24);
}

Size PanelWx::getMaximumSize() const
{
	return WidgetWxImpl< IPanel, wxPanel >::getMaximumSize() + Size(16, 24);
}

	}
}
