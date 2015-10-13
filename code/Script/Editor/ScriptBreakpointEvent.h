#ifndef traktor_script_ScriptBreakpointEvent_H
#define traktor_script_ScriptBreakpointEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace script
	{

class StackFrame;

class ScriptBreakpointEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	ScriptBreakpointEvent(ui::EventSubject* sender, const StackFrame* currentFrame);

	const StackFrame* getCurrentFrame() const { return m_currentFrame; }

private:
	Ref< const StackFrame > m_currentFrame;
};

	}
}

#endif	// traktor_script_ScriptBreakpointEvent_H
