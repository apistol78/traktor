#ifndef traktor_amalgam_TargetScriptDebugger_H
#define traktor_amalgam_TargetScriptDebugger_H

#include <list>
#include "Script/IScriptDebugger.h"

namespace traktor
{
	namespace amalgam
	{

class TargetConnection;

class TargetScriptDebugger : public script::IScriptDebugger
{
	T_RTTI_CLASS;

public:
	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool isRunning();

	virtual bool actionBreak();

	virtual bool actionContinue();

	virtual bool actionStepInto();

	virtual bool actionStepOver();

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

	void addConnection(TargetConnection* connection);

	void removeConnection(TargetConnection* connection);

	const std::list< std::pair< Guid, int32_t > >& getBreakpoints() const { return m_breakpoints; }

	void notifyListeners(const script::CallStack& callStack);

private:
	std::list< std::pair< Guid, int32_t > > m_breakpoints;
	std::list< IListener* > m_listeners;
	std::list< TargetConnection* > m_connections;
};

	}
}

#endif	// traktor_amalgam_TargetScriptDebugger_H
