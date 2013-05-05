#include "Core/Log/Log.h"
#include "Core/Thread/Acquire.h"
#include "Amalgam/Editor/TargetConnection.h"
#include "Amalgam/Editor/TargetScriptDebugger.h"
#include "Amalgam/Editor/TargetScriptDebuggerSessions.h"
#include "Amalgam/Impl/ScriptDebuggerHalted.h"
#include "Amalgam/Impl/TargetLog.h"
#include "Net/BidirectionalObjectTransport.h"

namespace traktor
{
	namespace amalgam
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.amalgam.TargetConnection", TargetConnection, Object)

TargetConnection::TargetConnection(net::BidirectionalObjectTransport* transport, ILogTarget* targetLog, TargetScriptDebuggerSessions* targetDebuggerSessions)
:	m_transport(transport)
,	m_targetLog(targetLog)
,	m_targetDebuggerSessions(targetDebuggerSessions)
{
	m_targetDebugger = new TargetScriptDebugger(m_transport);
	m_targetDebuggerSessions->beginSession(m_targetDebugger);
}

TargetConnection::~TargetConnection()
{
	T_ASSERT (!m_targetDebugger);
	T_ASSERT (!m_transport);
}

void TargetConnection::destroy()
{
	T_ANONYMOUS_VAR(Acquire< Semaphore >)(m_lock);

	if (m_targetDebugger)
	{
		T_ASSERT (m_targetDebuggerSessions);
		m_targetDebuggerSessions->endSession(m_targetDebugger);
		m_targetDebuggerSessions = 0;
		m_targetDebugger = 0;
	}

	m_targetLog = 0;

	if (m_transport)
	{
		m_transport->close();
		m_transport = 0;
	}
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
		destroy();
		return false;
	}

	{
		Ref< TargetPerformance > performance;
		if (m_transport->recv< TargetPerformance >(0, performance) == net::BidirectionalObjectTransport::RtSuccess)
		{
			m_performance = *performance;
			m_transport->flush< TargetPerformance >();
		}
	}

	{
		Ref< TargetLog > tlog;
		while (m_transport->recv< TargetLog >(0, tlog) == net::BidirectionalObjectTransport::RtSuccess)
			m_targetLog->log(tlog->getLevel(), tlog->getText());
	}

	{
		Ref< ScriptDebuggerHalted > debugger;
		while (m_transport->recv< ScriptDebuggerHalted >(0, debugger) == net::BidirectionalObjectTransport::RtSuccess)
		{
			const script::CallStack& cs = debugger->getCallStack();
			m_targetDebugger->notifyListeners(cs);
		}
	}

	return true;
}

	}
}
