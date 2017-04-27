/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/TabPage.h"
#include "Ui/Tab.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabPage", TabPage, Container)

bool TabPage::create(Tab* tab, const std::wstring& text, int imageIndex, Layout* layout)
{
	m_tab = tab;
	m_imageIndex = imageIndex;

	if (!Container::create(tab, WsNone, layout))
		return false;

	setText(text);

	return true;
}

bool TabPage::create(Tab* tab, const std::wstring& text, Layout* layout)
{
	return create(tab, text, 0, layout);
}

void TabPage::setActive()
{
	m_tab->setActivePage(this);
}

bool TabPage::isActive() const
{
	return bool(m_tab->getActivePage() == this);
}

Ref< Tab > TabPage::getTab()
{
	return m_tab;
}

int TabPage::getImageIndex() const
{
	return m_imageIndex;
}

	}
}
