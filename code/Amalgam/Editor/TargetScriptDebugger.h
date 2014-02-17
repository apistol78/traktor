#ifndef traktor_amalgam_TargetScriptDebugger_H
#define traktor_amalgam_TargetScriptDebugger_H

#include <list>
#include "Script/IScriptDebugger.h"

namespace traktor
{
	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace amalgam
	{

class TargetScriptDebugger : public script::IScriptDebugger
{
	T_RTTI_CLASS;

public:
	TargetScriptDebugger(net::BidirectionalObjectTransport* transport);

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber);

	virtual bool isRunning() const;

	virtual bool actionBreak();

	virtual bool actionContinue();

	virtual bool actionStepInto();

	virtual bool actionStepOver();

	virtual void addListener(IListener* listener);

	virtual void removeListener(IListener* listener);

	void notifyListeners(const script::CallStack& callStack);

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
	Ref< script::CallStack > m_currentCallStack;
};

	}
}

#endif	// traktor_amalgam_TargetScriptDebugger_H
