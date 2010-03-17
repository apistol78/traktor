#ifndef traktor_db_RemoteBus_H
#define traktor_db_RemoteBus_H

#include "Database/Provider/IProviderBus.h"

namespace traktor
{
	namespace db
	{

class RemoteConnection;

/*! \brief Remote event bus.
 * \ingroup Database
 */
class RemoteBus : public IProviderBus
{
	T_RTTI_CLASS;

public:
	RemoteBus(RemoteConnection* connection, uint32_t handle);

	virtual ~RemoteBus();

	virtual bool putEvent(ProviderEvent event, const Guid& eventId);

	virtual bool getEvent(ProviderEvent& outEvent, Guid& outEventId, bool& outRemote);

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

	}
}

#endif	// traktor_db_RemoteBus_H
