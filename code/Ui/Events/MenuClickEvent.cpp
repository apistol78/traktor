/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/MenuClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.MenuClickEvent", MenuClickEvent, Event)

MenuClickEvent::MenuClickEvent(EventSubject* sender, MenuItem* item, const Command& command)
:	Event(sender)
,	m_item(item)
,	m_command(command)
{
}

MenuItem* MenuClickEvent::getItem() const
{
	return m_item;
}

const Command& MenuClickEvent::getCommand() const
{
	return m_command;
}

	}
}
