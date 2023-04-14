/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Runtime/Editor/Ui/TargetCommandEvent.h"

namespace traktor::runtime
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.runtime.TargetCommandEvent", TargetCommandEvent, ui::Event)

TargetCommandEvent::TargetCommandEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex, const std::wstring& command)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
,	m_command(command)
{
}

TargetInstance* TargetCommandEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetCommandEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

const std::wstring& TargetCommandEvent::getCommand() const
{
	return m_command;
}

}
