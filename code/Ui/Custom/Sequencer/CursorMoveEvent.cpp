/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Sequencer/CursorMoveEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.CursorMoveEvent", CursorMoveEvent, Event)

CursorMoveEvent::CursorMoveEvent(EventSubject* sender, int32_t position)
:	Event(sender)
,	m_position(position)
{
}

int32_t CursorMoveEvent::getPosition() const
{
	return m_position;
}

		}
	}
}
