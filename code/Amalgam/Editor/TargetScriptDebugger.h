#pragma once

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

/*! \brief
 * \ingroup Amalgam
 */
class TargetScriptDebugger : public script::IScriptDebugger
{
	T_RTTI_CLASS;

public:
	TargetScriptDebugger(net::BidirectionalObjectTransport* transport);

	void update();

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) override final;

	virtual bool captureStackFrame(uint32_t depth, Ref< script::StackFrame >& outStackFrame) override final;

	virtual bool captureLocals(uint32_t depth, RefArray< script::Variable >& outLocals) override final;

	virtual bool captureObject(uint32_t object, RefArray< script::Variable >& outMembers) override final;

	virtual bool isRunning() const override final;

	virtual bool actionBreak() override final;

	virtual bool actionContinue() override final;

	virtual bool actionStepInto() override final;

	virtual bool actionStepOver() override final;

	virtual void addListener(IListener* listener) override final;

	virtual void removeListener(IListener* listener) override final;

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;
	bool m_running;

	void notifyListeners();
};

	}
}

