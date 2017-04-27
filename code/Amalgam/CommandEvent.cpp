/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/CommandEvent.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.CommandEvent", 0, CommandEvent, IRemoteEvent)

CommandEvent::CommandEvent()
{
}

CommandEvent::CommandEvent(const std::wstring& function)
:	m_function(function)
{
}

const std::wstring& CommandEvent::getFunction() const
{
	return m_function;
}

void CommandEvent::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"function", m_function);
}

	}
}
