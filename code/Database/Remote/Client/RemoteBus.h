#pragma once

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

	virtual bool putEvent(const IEvent* event) override final;

	virtual bool getEvent(Ref< const IEvent >& outEvent, bool& outRemote) override final;

private:
	Ref< RemoteConnection > m_connection;
	uint32_t m_handle;
};

	}
}

