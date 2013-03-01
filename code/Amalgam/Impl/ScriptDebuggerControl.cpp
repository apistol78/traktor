#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberEnum.h"
#include "Amalgam/Impl/ScriptDebuggerControl.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.amalgam.ScriptDebuggerControl", 0, ScriptDebuggerControl, ISerializable)

ScriptDebuggerControl::ScriptDebuggerControl()
:	m_action(AcBreak)
{
}

ScriptDebuggerControl::ScriptDebuggerControl(Action action)
:	m_action(action)
{
}

bool ScriptDebuggerControl::serialize(ISerializer& s)
{
	const MemberEnum< Action >::Key c_Action_Keys[] =
	{
		{ L"AcBreak", AcBreak },
		{ L"AcContinue", AcContinue },
		{ L"AcStepInto", AcStepInto },
		{ L"AcStepOver", AcStepOver },
		{ 0 }
	};
	s >> MemberEnum< Action >(L"action", m_action, c_Action_Keys);
	return true;
}

	}
}
