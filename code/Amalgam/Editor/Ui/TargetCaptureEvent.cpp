/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/Ui/TargetCaptureEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetCaptureEvent", TargetCaptureEvent, ui::Event)

TargetCaptureEvent::TargetCaptureEvent(ui::EventSubject* sender, TargetInstance* instance, int32_t connectionIndex)
:	Event(sender)
,	m_instance(instance)
,	m_connectionIndex(connectionIndex)
{
}

TargetInstance* TargetCaptureEvent::getInstance() const
{
	return m_instance;
}

int32_t TargetCaptureEvent::getConnectionIndex() const
{
	return m_connectionIndex;
}

	}
}
