#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Impl/ScriptDebuggerHalted.h"
#include "Core/Thread/Acquire.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetConnection", TargetConnection, Object)

TargetConnection::TargetConnection(net::BidirectionalObjectTransport* transport, TargetScriptDebugger* targetDebugger)
:	m_transport(transport)
,	m_targetDebugger(targetDebugger)
{
	m_targetDebugger->addConnection(this);
}

void TargetConnection::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);
	if (m_targetDebugger)
	{
		m_targetDebugger->removeConnection(this);
		m_targetDebugger = 0;
	}
	m_transport = 0;
}

void TargetConnection::shutdown()
{
	destroy();
}

bool TargetConnection::update()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (!m_transport)
		return false;

	if (!m_transport->connected())
	{
		m_transport = 0;
		return false;
	}

	{
		Ref< TargetPerformance > performance;
		if (m_transport->recv< TargetPerformance >(0, performance) == net::BidirectionalObjectTransport::RtSuccess)
			m_performance = *performance;
	}

	{
		Ref< ScriptDebuggerHalted > debugger;
		if (m_transport->recv< ScriptDebuggerHalted >(0, debugger) == net::BidirectionalObjectTransport::RtSuccess)
		{
			const script::CallStack& cs = debugger->getCallStack();
			m_targetDebugger->notifyListeners(cs);
		}
	}

	return true;
}

	}
}
