/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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
