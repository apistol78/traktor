/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Events/TabCloseEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.TabCloseEvent", TabCloseEvent, CloseEvent)

TabCloseEvent::TabCloseEvent(EventSubject* sender, TabPage* tabPage)
:	CloseEvent(sender)
,	m_tabPage(tabPage)
{
}

TabPage* TabCloseEvent::getTabPage() const
{
	return m_tabPage;
}

	}
}
