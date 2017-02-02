#ifndef traktor_db_GroupMessageListener_H
#define traktor_db_GroupMessageListener_H

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Group message listener.
 * \ingroup Database
 */
class GroupMessageListener : public IMessageListenerImpl< GroupMessageListener >
{
	T_RTTI_CLASS;

public:
	GroupMessageListener(Connection* connection);

private:
	Connection* m_connection;

	bool messageGetGroupName(const class DbmGetGroupName* message);

	bool messageRenameGroup(const class DbmRenameGroup* message);

	bool messageRemoveGroup(const class DbmRemoveGroup* message);

	bool messageCreateGroup(const class DbmCreateGroup* message);

	bool messageCreateInstance(const class DbmCreateInstance* message);

	bool messageGetChildren(const class DbmGetChildren* message);
};

	}
}

#endif	// traktor_db_GroupMessageListener_H
