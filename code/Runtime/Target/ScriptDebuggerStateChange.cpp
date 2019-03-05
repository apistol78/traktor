#include "Runtime/Target/ScriptDebuggerStateChange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerStateChange", 0, ScriptDebuggerStateChange, ISerializable)

ScriptDebuggerStateChange::ScriptDebuggerStateChange()
:	m_running(false)
{
}

ScriptDebuggerStateChange::ScriptDebuggerStateChange(bool running)
:	m_running(running)
{
}

void ScriptDebuggerStateChange::serialize(ISerializer& s)
{
	s >> Member< bool >(L"running", m_running);
}

	}
}
