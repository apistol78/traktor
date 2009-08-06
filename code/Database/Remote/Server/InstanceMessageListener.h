#ifndef traktor_db_InstanceMessageListener_H
#define traktor_db_InstanceMessageListener_H

#include "Core/Heap/Ref.h"
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief
 */
class InstanceMessageListener : public IMessageListenerImpl< InstanceMessageListener >
{
	T_RTTI_CLASS(InstanceMessageListener)

public:
	InstanceMessageListener(Connection* connection);

private:
	Ref< Connection > m_connection;

	bool messageGetInstancePrimaryType(const class DbmGetInstancePrimaryType* message);

	bool messageOpenTransaction(const class DbmOpenTransaction* message);

	bool messageCommitTransaction(const class DbmCommitTransaction* message);

	bool messageCloseTransaction(const class DbmCloseTransaction* message);

	bool messageGetInstanceName(const class DbmGetInstanceName* message);

	bool messageSetInstanceName(const class DbmSetInstanceName* message);

	bool messageGetInstanceGuid(const class DbmGetInstanceGuid* message);

	bool messageSetInstanceGuid(const class DbmSetInstanceGuid* message);

	bool messageRemoveInstance(const class DbmRemoveInstance* message);

	bool messageReadObject(const class DbmReadObject* message);

	bool messageWriteObject(const class DbmWriteObject* message);
};

	}
}

#endif	// traktor_db_InstanceMessageListener_H
