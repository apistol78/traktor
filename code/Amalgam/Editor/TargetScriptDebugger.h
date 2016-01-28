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

/*! \brief
 * \ingroup Amalgam
 */
class TargetScriptDebugger : public script::IScriptDebugger
{
	T_RTTI_CLASS;

public:
	TargetScriptDebugger(net::BidirectionalObjectTransport* transport);

	void update();

	virtual bool setBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual bool removeBreakpoint(const Guid& scriptId, int32_t lineNumber) T_OVERRIDE T_FINAL;

	virtual Ref< script::StackFrame > captureStackFrame(uint32_t depth) T_OVERRIDE T_FINAL;

	virtual bool isRunning() const T_OVERRIDE T_FINAL;

	virtual bool actionBreak() T_OVERRIDE T_FINAL;

	virtual bool actionContinue() T_OVERRIDE T_FINAL;

	virtual bool actionStepInto() T_OVERRIDE T_FINAL;

	virtual bool actionStepOver() T_OVERRIDE T_FINAL;

	virtual void addListener(IListener* listener) T_OVERRIDE T_FINAL;

	virtual void removeListener(IListener* listener) T_OVERRIDE T_FINAL;

private:
	Ref< net::BidirectionalObjectTransport > m_transport;
	std::list< IListener* > m_listeners;

	void notifyListeners();
};

	}
}

#endif	// traktor_amalgam_TargetScriptDebugger_H
