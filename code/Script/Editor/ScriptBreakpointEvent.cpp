#include "Script/Editor/ScriptBreakpointEvent.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptBreakpointEvent", ScriptBreakpointEvent, ui::Event)

ScriptBreakpointEvent::ScriptBreakpointEvent(ui::EventSubject* sender, const CallStack* callStack)
:	Event(sender)
,	m_callStack(callStack)
{
}

const CallStack* ScriptBreakpointEvent::getCallStack() const
{
	return m_callStack;
}

	}
}
