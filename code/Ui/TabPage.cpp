/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Tab.h"
#include "Ui/TabPage.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabPage", TabPage, Container)

bool TabPage::create(Tab* tab, const std::wstring& text, int32_t imageIndex, Layout* layout)
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

Tab* TabPage::getTab() const
{
	return m_tab;
}

int32_t TabPage::getImageIndex() const
{
	return m_imageIndex;
}

}
