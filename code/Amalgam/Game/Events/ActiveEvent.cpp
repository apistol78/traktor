/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/Game/Events/ActiveEvent.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.ActiveEvent", ActiveEvent, Object)

ActiveEvent::ActiveEvent(bool activated)
:	m_activated(activated)
{
}

bool ActiveEvent::becameActivated() const
{
	return m_activated;
}

	}
}
