/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Graph/GroupMovedEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GroupMovedEvent", GroupMovedEvent, Event)

GroupMovedEvent::GroupMovedEvent(EventSubject* sender, Group* group)
:	Event(sender)
,	m_group(group)
{
}

Group* GroupMovedEvent::getGroup() const
{
	return m_group;
}

}
