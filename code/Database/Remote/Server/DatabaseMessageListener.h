#ifndef traktor_db_DatabaseMessageListener_H
#define traktor_db_DatabaseMessageListener_H

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Configuration;
class Connection;

/*! \brief Database message listener.
 * \ingroup Database
 */
class DatabaseMessageListener : public IMessageListenerImpl< DatabaseMessageListener >
{
	T_RTTI_CLASS;

public:
	DatabaseMessageListener(const Configuration* configuration, Connection* connection);

private:
	const Configuration* m_configuration;
	Connection* m_connection;

	bool messageOpen(const class DbmOpen* message);

	bool messageClose(const class DbmClose* message);

	bool messageGetBus(const class DbmGetBus* message);

	bool messageGetRootGroup(const class DbmGetRootGroup* message);
};

	}
}

#endif	// traktor_db_DatabaseMessageListener_H
