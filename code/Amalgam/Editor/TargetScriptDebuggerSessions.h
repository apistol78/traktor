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

/*! \brief
 * \ingroup Amalgam
 */
class TargetScriptDebuggerSessions : public script::IScriptDebuggerSessions
{
	T_RTTI_CLASS;

public:
	void beginSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler);

	void endSession(TargetScriptDebugger* scriptDebugger, TargetScriptProfiler* scriptProfiler);

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual bool removeAllBreakpoints(const Guid& scriptId) T_OVERRIDE T_FINAL;

	virtual bool haveBreakpoint(const Guid& scriptId, int32_t lineNumber) const T_OVERRIDE T_FINAL;

	virtual void addListener(IListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(IListener* listener) T_OVERRIDE T_FINAL;

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
