#ifndef traktor_script_ScriptBreakpointEvent_H
#define traktor_script_ScriptBreakpointEvent_H

#include "Ui/Event.h"

namespace traktor
{
	namespace script
	{

class CallStack;

class ScriptBreakpointEvent : public ui::Event
{
	T_RTTI_CLASS;

public:
	ScriptBreakpointEvent(ui::EventSubject* sender, const CallStack* callStack);

	const CallStack* getCallStack() const;

private:
	Ref< const CallStack > m_callStack;
};

	}
}

#endif	// traktor_script_ScriptBreakpointEvent_H
