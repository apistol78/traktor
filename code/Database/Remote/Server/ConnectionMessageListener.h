#pragma once

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! Connection message listener.
 * \ingroup Database
 */
class ConnectionMessageListener : public IMessageListenerImpl< ConnectionMessageListener >
{
	T_RTTI_CLASS;

public:
	ConnectionMessageListener(Connection* connection);

private:
	Connection* m_connection;

	bool messageReleaseObject(const class CnmReleaseObject* message);
};

	}
}

