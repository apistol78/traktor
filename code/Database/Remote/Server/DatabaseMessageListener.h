/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Core/Containers/SmallMap.h"
#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{

class Semaphore;

	namespace db
	{

class Connection;

/*! Database message listener.
 * \ingroup Database
 */
class DatabaseMessageListener : public IMessageListenerImpl< DatabaseMessageListener >
{
	T_RTTI_CLASS;

public:
	DatabaseMessageListener(
		Semaphore& connectionStringsLock,
		const SmallMap< std::wstring, std::wstring >& connectionStrings,
		uint16_t streamServerPort,
		Connection* connection
	);

private:
	Semaphore& m_connectionStringsLock;
	const SmallMap< std::wstring, std::wstring >& m_connectionStrings;
	uint16_t m_streamServerPort;
	Connection* m_connection;

	bool messageOpen(const class DbmOpen* message);

	bool messageClose(const class DbmClose* message);

	bool messageGetBus(const class DbmGetBus* message);

	bool messageGetRootGroup(const class DbmGetRootGroup* message);
};

	}
}

