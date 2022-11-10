/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/TargetPlayEvent.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetPlayEvent", TargetPlayEvent, ui::Event)

TargetPlayEvent::TargetPlayEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetPlayEvent::getInstance() const
{
	return m_instance;
}

	}
}
