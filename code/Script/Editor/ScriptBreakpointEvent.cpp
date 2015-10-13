#include "Script/Editor/ScriptBreakpointEvent.h"

namespace traktor
{
	namespace script
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.script.ScriptBreakpointEvent", ScriptBreakpointEvent, ui::Event)

ScriptBreakpointEvent::ScriptBreakpointEvent(ui::EventSubject* sender, const StackFrame* currentFrame)
:	Event(sender)
,	m_currentFrame(currentFrame)
{
}

	}
}
