#include "Runtime/Target/ScriptDebuggerStackFrame.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Script/StackFrame.h"

namespace traktor
{
	namespace runtime
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.runtime.ScriptDebuggerStackFrame", 0, ScriptDebuggerStackFrame, ISerializable)

ScriptDebuggerStackFrame::ScriptDebuggerStackFrame(script::StackFrame* frame)
:	m_frame(frame)
{
}

void ScriptDebuggerStackFrame::serialize(ISerializer& s)
{
	s >> MemberRef< script::StackFrame >(L"frame", m_frame);
}

	}
}
