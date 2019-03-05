#include "Runtime/Target/ScriptDebuggerStatus.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerStatus", 0, ScriptDebuggerStatus, ISerializable)

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
