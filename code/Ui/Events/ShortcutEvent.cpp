/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Events/ShortcutEvent.h"

namespace traktor
{
	namespace ui
	{
	
T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.ShortcutEvent", ShortcutEvent, Event)

ShortcutEvent::ShortcutEvent(EventSubject* sender, const Command& command)
:	Event(sender)
,	m_command(command)
{
}

const Command& ShortcutEvent::getCommand() const
{
	return m_command;
}

	}
}
