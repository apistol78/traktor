/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
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

namespace traktor
{
	namespace db
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
	Ref< MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetGroupName(m_handle));
	return result ? result->get() : L"";
}

bool RemoteGroup::rename(const std::wstring& name)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRenameGroup(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteGroup::remove()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveGroup(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Ref< IProviderGroup > RemoteGroup::createGroup(const std::wstring& groupName)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateGroup(m_handle, groupName));
	return result ? new RemoteGroup(m_connection, result->get()) : 0;
}

Ref< IProviderInstance > RemoteGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateInstance(m_handle, instanceName, instanceGuid));
	return result ? new RemoteInstance(m_connection, result->get()) : 0;
}

bool RemoteGroup::getChildren(RefArray< IProviderGroup >& outChildGroups, RefArray< IProviderInstance >& outChildInstances)
{
	Ref< MsgGetChildrenResult > result = m_connection->sendMessage< MsgGetChildrenResult >(DbmGetChildren(m_handle));
	if (!result)
		return false;

	for (std::vector< uint32_t >::const_iterator i = result->getGroups().begin(); i != result->getGroups().end(); ++i)
		outChildGroups.push_back(new RemoteGroup(m_connection, *i));

	for (std::vector< uint32_t >::const_iterator i = result->getInstances().begin(); i != result->getInstances().end(); ++i)
		outChildInstances.push_back(new RemoteInstance(m_connection, *i));

	return true;
}

	}
}
