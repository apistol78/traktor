/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Ui/Sequencer/GroupVisibleEvent.h"

namespace traktor::ui
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.GroupVisibleEvent", GroupVisibleEvent, Event)

GroupVisibleEvent::GroupVisibleEvent(EventSubject* sender, SequenceGroup* group, bool visible)
:	Event(sender)
,	m_group(group)
,	m_visible(visible)
{
}

SequenceGroup* GroupVisibleEvent::getGroup() const
{
	return m_group;
}

bool GroupVisibleEvent::getVisible() const
{
	return m_visible;
}

}
