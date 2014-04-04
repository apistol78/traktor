#ifndef traktor_amalgam_TargetConnection_H
#define traktor_amalgam_TargetConnection_H

#include "Amalgam/Impl/TargetPerformance.h"
#include "Core/Object.h"
#include "Core/Thread/Semaphore.h"

namespace traktor
{

class ILogTarget;

	namespace net
	{

class BidirectionalObjectTransport;

	}

	namespace amalgam
	{

class TargetScriptDebugger;
class TargetScriptDebuggerSessions;
class TargetScriptProfiler;

class TargetConnection : public Object
{
	T_RTTI_CLASS;

public:
	TargetConnection(const std::wstring& name, net::BidirectionalObjectTransport* transport, ILogTarget* targetLog, TargetScriptDebuggerSessions* targetDebuggerSessions);

	virtual ~TargetConnection();

	void destroy();

	void shutdown();

	bool update();

	const std::wstring& getName() const { return m_name; }

	net::BidirectionalObjectTransport* getTransport() const { return m_transport; }

	const TargetPerformance& getPerformance() const { return m_performance; }

private:
	std::wstring m_name;
	Ref< net::BidirectionalObjectTransport > m_transport;
	Ref< ILogTarget > m_targetLog;
	Ref< TargetScriptDebuggerSessions > m_targetDebuggerSessions;
	Ref< TargetScriptDebugger > m_targetDebugger;
	Ref< TargetScriptProfiler > m_targetProfiler;
	TargetPerformance m_performance;
	Semaphore m_lock;
};

	}
}

#endif	// traktor_amalgam_TargetConnection_H
