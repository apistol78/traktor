#ifndef traktor_amalgam_TargetScriptDebuggerSessions_H
#define traktor_amalgam_TargetScriptDebuggerSessions_H

#include <list>
#include <map>
#include "Core/RefArray.h"
#include "Script/Editor/IScriptDebuggerSessions.h"

namespace traktor
{
	namespace amalgam
	{

class TargetScriptDebugger;

class TargetScriptDebuggerSessions : public script::IScriptDebuggerSessions
{
	T_RTTI_CLASS;

public:
	void beginSession(TargetScriptDebugger* scriptDebugger);

	void endSession(TargetScriptDebugger* scriptDebugger);

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const;

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

private:
	RefArray< TargetScriptDebugger > m_scriptDebuggers;
	std::map< int32_t, std::set< Guid > > m_breakpoints;
	std::list< IListener* > m_listeners;
};

	}
}


#endif	// traktor_amalgam_TargetScriptDebuggerSessions_H
