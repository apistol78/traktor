/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/ScriptDebuggerStatus.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerStatus", 0, ScriptDebuggerStatus, ISerializable)

ScriptDebuggerStatus::ScriptDebuggerStatus()
{
}

ScriptDebuggerStatus::ScriptDebuggerStatus(bool running)
:	m_running(running)
{
}

void ScriptDebuggerStatus::serialize(ISerializer& s)
{
	s >> Member< bool >(L"running", m_running);
}

	}
}
