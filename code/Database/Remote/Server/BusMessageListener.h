#ifndef traktor_db_BusMessageListener_H
#define traktor_db_BusMessageListener_H

#include "Core/Heap/Ref.h"
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief
 */
class BusMessageListener : public IMessageListenerImpl< BusMessageListener >
{
	T_RTTI_CLASS(BusMessageListener)

public:
	BusMessageListener(Connection* connection);

private:
	Ref< Connection > m_connection;

	bool messagePutEvent(const class DbmPutEvent* message);

	bool messageGetEvent(const class DbmGetEvent* message);
};

	}
}

#endif	// traktor_db_BusMessageListener_H
