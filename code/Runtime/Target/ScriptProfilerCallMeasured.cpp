#include "Runtime/Target/ScriptProfilerCallMeasured.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptProfilerCallMeasured", 0, ScriptProfilerCallMeasured, ISerializable)

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
