/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#pragma once

#include "Database/Remote/Server/IMessageListenerImpl.h"

namespace traktor
{
	namespace db
	{

class Connection;

/*! Event bus message listener.
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

