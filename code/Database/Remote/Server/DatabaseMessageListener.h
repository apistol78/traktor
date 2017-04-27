/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_db_DatabaseMessageListener_H
#define traktor_db_DatabaseMessageListener_H

#include <map>
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{

class Semaphore;

	namespace db
	{

class Connection;

/*! \brief Database message listener.
 * \ingroup Database
 */
class DatabaseMessageListener : public IMessageListenerImpl< DatabaseMessageListener >
{
	T_RTTI_CLASS;

public:
	DatabaseMessageListener(
		Semaphore& connectionStringsLock,
		const std::map< std::wstring, std::wstring >& connectionStrings,
		uint16_t streamServerPort,
		Connection* connection
	);

private:
	Semaphore& m_connectionStringsLock;
	const std::map< std::wstring, std::wstring >& m_connectionStrings;
	uint16_t m_streamServerPort;
	Connection* m_connection;

	bool messageOpen(const class DbmOpen* message);

	bool messageClose(const class DbmClose* message);

	bool messageGetBus(const class DbmGetBus* message);

	bool messageGetRootGroup(const class DbmGetRootGroup* message);
};

	}
}

#endif	// traktor_db_DatabaseMessageListener_H
