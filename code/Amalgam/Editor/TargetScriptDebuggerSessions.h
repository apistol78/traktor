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
class TargetScriptProfiler;

class TargetScriptDebuggerSessions : public script::IScriptDebuggerSessions
{
	T_RTTI_CLASS;

public:
	void beginSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler);

	void endSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler);

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeAllBreakpoints(const Guid& scriptId);

	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const;

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

private:
	struct Session
	{
		Ref< TargetScriptDebugger > debugger;
		Ref< TargetScriptProfiler > profiler;
	};
	std::list< Session > m_sessions;
	std::map< int32_t, std::set< Guid > > m_breakpoints;
	std::list< IListener* > m_listeners;
};

	}
}


#endif	// traktor_amalgam_TargetScriptDebuggerSessions_H
