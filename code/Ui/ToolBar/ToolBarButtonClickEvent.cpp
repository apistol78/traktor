/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/ToolBar/ToolBarButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ToolBarButtonClickEvent", ToolBarButtonClickEvent, ButtonClickEvent)

ToolBarButtonClickEvent::ToolBarButtonClickEvent(EventSubject* sender, ToolBarItem* item, const Command& command, const MenuItem* menuItem)
:	ButtonClickEvent(sender, command)
,	m_item(item)
,	m_menuItem(menuItem)
{
}

ToolBarItem* ToolBarButtonClickEvent::getItem() const
{
	return m_item;
}

const MenuItem* ToolBarButtonClickEvent::getMenuItem() const
{
	return m_menuItem;
}

	}
}
