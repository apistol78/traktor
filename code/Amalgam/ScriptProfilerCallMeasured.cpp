/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Amalgam/ScriptProfilerCallMeasured.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptProfilerCallMeasured", 0, ScriptProfilerCallMeasured, ISerializable)

ScriptProfilerCallMeasured::ScriptProfilerCallMeasured()
:	m_callCount(0)
,	m_inclusiveDuration(0.0)
,	m_exclusiveDuration(0.0)
{
}

ScriptProfilerCallMeasured::ScriptProfilerCallMeasured(const Guid& scriptId, const std::wstring& function, uint32_t callCount, double inclusiveDuration, double exclusiveDuration)
:	m_scriptId(scriptId)
,	m_function(function)
,	m_callCount(callCount)
,	m_inclusiveDuration(inclusiveDuration)
,	m_exclusiveDuration(exclusiveDuration)
{
}

void ScriptProfilerCallMeasured::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"scriptId", m_scriptId);
	s >> Member< std::wstring >(L"function", m_function);
	s >> Member< uint32_t >(L"callCount", m_callCount);
	s >> Member< double >(L"inclusiveDuration", m_inclusiveDuration);
	s >> Member< double >(L"exclusiveDuration", m_exclusiveDuration);
}

	}
}
