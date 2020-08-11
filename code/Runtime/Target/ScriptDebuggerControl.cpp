#include "Runtime/Target/ScriptDebuggerControl.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberEnum.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerControl", 0, ScriptDebuggerControl, ISerializable)

ScriptDebuggerControl::ScriptDebuggerControl(Action action)
:	m_action(action)
,	m_param(0)
{
}

ScriptDebuggerControl::ScriptDebuggerControl(Action action, uint32_t param)
:	m_action(action)
,	m_param(param)
{
}

void ScriptDebuggerControl::serialize(ISerializer& s)
{
	s >> MemberEnumByValue< Action, uint8_t >(L"action", m_action);
	s >> Member< uint32_t >(L"param", m_param);
}

	}
}
