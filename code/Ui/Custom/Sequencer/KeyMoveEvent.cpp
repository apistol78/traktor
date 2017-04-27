/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Custom/Sequencer/KeyMoveEvent.h"

namespace traktor
{
	namespace ui
	{
		namespace custom
		{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.custom.KeyMoveEvent", KeyMoveEvent, Event)

KeyMoveEvent::KeyMoveEvent(EventSubject* sender, Key* key, int32_t offset)
:	Event(sender)
,	m_key(key)
,	m_offset(offset)
{
}

Key* KeyMoveEvent::getKey() const
{
	return m_key;
}

int32_t KeyMoveEvent::getOffset() const
{
	return m_offset;
}

		}
	}
}
