#ifndef traktor_db_DatabaseMessageListener_H
#define traktor_db_DatabaseMessageListener_H

#include "Core/Heap/Ref.h"
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Configuration;
class Connection;

/*! \brief
 */
class DatabaseMessageListener : public IMessageListenerImpl< DatabaseMessageListener >
{
	T_RTTI_CLASS(DatabaseMessageListener)

public:
	DatabaseMessageListener(const Configuration* configuration, Connection* connection);

private:
	Ref< const Configuration > m_configuration;
	Ref< Connection > m_connection;

	bool messageOpen(const class DbmOpen* message);

	bool messageClose(const class DbmClose* message);

	bool messageGetRootGroup(const class DbmGetRootGroup* message);
};

	}
}

#endif	// traktor_db_DatabaseMessageListener_H
