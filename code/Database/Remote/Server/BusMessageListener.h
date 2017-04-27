/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_BusMessageListener_H
#define traktor_db_BusMessageListener_H

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! \brief Event bus message listener.
 * \ingroup Database
 */
class BusMessageListener : public IMessageListenerImpl< BusMessageListener >
{
	T_RTTI_CLASS;

public:
	BusMessageListener(Connection* connection);

private:
	Connection* m_connection;

	bool messagePutEvent(const class DbmPutEvent* message);

	bool messageGetEvent(const class DbmGetEvent* message);
};

	}
}

#endif	// traktor_db_BusMessageListener_H
