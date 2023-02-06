/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Database/Remote/Client/RemoteGroup.h"
#include "Database/Remote/Client/RemoteInstance.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/DbmGetGroupName.h"
#include "Database/Remote/Messages/DbmRenameGroup.h"
#include "Database/Remote/Messages/DbmRemoveGroup.h"
#include "Database/Remote/Messages/DbmCreateGroup.h"
#include "Database/Remote/Messages/DbmCreateInstance.h"
#include "Database/Remote/Messages/DbmGetChildren.h"
#include "Database/Remote/Messages/MsgGetChildrenResult.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/MsgHandleArrayResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteGroup", RemoteGroup, IProviderGroup)

RemoteGroup::RemoteGroup(RemoteConnection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteGroup::~RemoteGroup()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

std::wstring RemoteGroup::getName() const
{
	Ref< const MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetGroupName(m_handle));
	return result ? result->get() : L"";
}

bool RemoteGroup::rename(const std::wstring& name)
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRenameGroup(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteGroup::remove()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveGroup(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Ref< IProviderGroup > RemoteGroup::createGroup(const std::wstring& groupName)
{
	Ref< const MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateGroup(m_handle, groupName));
	return result ? new RemoteGroup(m_connection, result->get()) : nullptr;
}

Ref< IProviderInstance > RemoteGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Ref< const MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateInstance(m_handle, instanceName, instanceGuid));
	return result ? new RemoteInstance(m_connection, result->get()) : nullptr;
}

bool RemoteGroup::getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances)
{
	Ref< MsgGetChildrenResult > result = m_connection->sendMessage< MsgGetChildrenResult >(DbmGetChildren(m_handle));
	if (!result)
		return false;

	for (auto group : result->getGroups())
		outChildGroups.push_back(new RemoteGroup(m_connection, group));

	for (auto instance : result->getInstances())
		outChildInstances.push_back(new RemoteInstance(m_connection, instance));

	return true;
}

}
