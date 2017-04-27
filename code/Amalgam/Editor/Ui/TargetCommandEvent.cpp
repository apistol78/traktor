/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/Ui/TargetCommandEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetCommandEvent", TargetCommandEvent, ui::Event)

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
}
