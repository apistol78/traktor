#ifndef traktor_db_LocalBus_H
#define traktor_db_LocalBus_H

#include <list>
#include "Database/Provider/IProviderBus.h"
#include "Core/Guid.h"
#include "Core/Thread/Mutex.h"

namespace traktor
{

class ISharedMemory;

	namespace db
	{

/*! \brief Local database event bus.
 * \ingroup Database
 *
 * Using named mutex to listen for new events across multiple
 * processes on the same machine.
 */
class LocalBus : public IProviderBus
{
	T_RTTI_CLASS;

public:
	LocalBus(const std::wstring& eventFile);

	virtual ~LocalBus();

	void close();

	virtual bool putEvent(const IEvent* event) T_OVERRIDE T_FINAL;

	virtual bool getEvent(Ref< const IEvent >& outEvent, bool& outRemote) T_OVERRIDE T_FINAL;

private:
	struct Event
	{
		Ref< const IEvent > event;
		bool remote;
	};

	Guid m_localGuid;
	Mutex m_globalLock;
	Ref< ISharedMemory > m_shm;
	std::list< Event > m_pendingEvents;
};

	}
}

#endif	// traktor_db_LocalBus_H
