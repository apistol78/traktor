#ifndef traktor_db_RemoteBus_H
#define traktor_db_RemoteBus_H

#include "Core/Heap/Ref.h"
#include "Database/Provider/IProviderBus.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Remote event bus.
 * \ingroup Database
 */
class RemoteBus : public IProviderBus
{
	T_RTTI_CLASS(RemoteBus)

public:
	RemoteBus(Connection* connection, uint32_t handle);

	virtual ~RemoteBus();

	virtual bool putEvent(ProviderEvent event, const Guid& eventId);

	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote);

private:
	Ref< Connection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteBus_H
