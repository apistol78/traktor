/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Editor/Ui/TargetMigrateEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetMigrateEvent", TargetMigrateEvent, ui::Event)

TargetMigrateEvent::TargetMigrateEvent(ui::EventSubject* sender, TargetInstance* instance)
:	Event(sender)
,	m_instance(instance)
{
}

TargetInstance* TargetMigrateEvent::getInstance() const
{
	return m_instance;
}

	}
}
