#include "Amalgam/Impl/ScriptDebuggerHalted.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerHalted", 0, ScriptDebuggerHalted, ISerializable)

ScriptDebuggerHalted::ScriptDebuggerHalted()
{
}

ScriptDebuggerHalted::ScriptDebuggerHalted(const script::CallStack& callStack)
:	m_callStack(callStack)
{
}

bool ScriptDebuggerHalted::serialize(ISerializer& s)
{
	s >> MemberComposite< script::CallStack >(L"callStack", m_callStack);
	return true;
}

	}
}
