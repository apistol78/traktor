#ifndef traktor_telemetry_TelemetryClient_H
#define traktor_telemetry_TelemetryClient_H

#include "Core/Object.h"
#include "Core/RefArray.h"
#include "Core/Thread/Semaphore.h"
#include "Core/Thread/Signal.h"
#include "Core/Thread/Thread.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_TELEMETRY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace telemetry
	{

class ITask;

/*! \brief
 * \ingroup Telemetry
 */
class T_DLLCLASS TelemetryClient : public Object
{
	T_RTTI_CLASS;

public:
	virtual ~TelemetryClient();

	static Ref< TelemetryClient > create(const std::wstring& serverHost, const std::wstring& client);

	void destroy();

	void set(const std::wstring& id, int32_t value);

	void add(const std::wstring& id, int32_t value);

	void flush();

protected:
	TelemetryClient();

private:
	Semaphore m_lock;
	Thread* m_thread;
	RefArray< ITask > m_queue;
	Signal m_queueSignal;
	std::wstring m_serverHost;
	std::wstring m_client;

	void threadProcessQueue();
};

	}
}

#endif	// traktor_telemetry_TelemetryClient_H
