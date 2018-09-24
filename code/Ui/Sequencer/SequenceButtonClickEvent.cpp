/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Ui/Sequencer/SequenceButtonClickEvent.h"

namespace traktor
{
	namespace ui
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.ui.SequenceButtonClickEvent", SequenceButtonClickEvent, ButtonClickEvent)

SequenceButtonClickEvent::SequenceButtonClickEvent(EventSubject* sender, Sequence* sequence, const Command& command)
:	ButtonClickEvent(sender, command)
,	m_sequence(sequence)
{
}

Sequence* SequenceButtonClickEvent::getSequence() const
{
	return m_sequence;
}

	}
}
