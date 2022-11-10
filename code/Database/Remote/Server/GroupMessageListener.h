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

/*! Group message listener.
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

