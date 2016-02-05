#include "Amalgam/ScriptDebuggerLocals.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRefArray.h"
#include "Script/Local.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerLocals", 0, ScriptDebuggerLocals, ISerializable)

ScriptDebuggerLocals::ScriptDebuggerLocals()
{
}

ScriptDebuggerLocals::ScriptDebuggerLocals(const RefArray< script::Local >& locals)
:	m_locals(locals)
{
}

void ScriptDebuggerLocals::serialize(ISerializer& s)
{
	s >> MemberRefArray< script::Local >(L"locals", m_locals);
}

	}
}
