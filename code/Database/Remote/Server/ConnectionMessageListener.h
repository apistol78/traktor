#ifndef traktor_db_ConnectionMessageListener_H
#define traktor_db_ConnectionMessageListener_H

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Connection message listener.
 * \ingroup Database
 */
class ConnectionMessageListener : public IMessageListenerImpl< ConnectionMessageListener >
{
	T_RTTI_CLASS;

public:
	ConnectionMessageListener(Connection* connection);

private:
	Ref< Connection > m_connection;

	bool messageReleaseObject(const class CnmReleaseObject* message);
};

	}
}

#endif	// traktor_db_ConnectionMessageListener_H
