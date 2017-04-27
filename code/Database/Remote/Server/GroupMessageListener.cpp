/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Provider/IProviderGroup.h"
#include "Database/Provider/IProviderInstance.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/GroupMessageListener.h"
#include "Database/Remote/Messages/DbmGetGroupName.h"
#include "Database/Remote/Messages/DbmRenameGroup.h"
#include "Database/Remote/Messages/DbmRemoveGroup.h"
#include "Database/Remote/Messages/DbmCreateGroup.h"
#include "Database/Remote/Messages/DbmCreateInstance.h"
#include "Database/Remote/Messages/DbmGetChildren.h"
#include "Database/Remote/Messages/MsgGetChildrenResult.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/MsgHandleArrayResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.GroupMessageListener", GroupMessageListener, IMessageListener)

GroupMessageListener::GroupMessageListener(Connection* connection)
:	m_connection(connection)
{
	registerMessage< DbmGetGroupName >(&GroupMessageListener::messageGetGroupName);
	registerMessage< DbmRenameGroup >(&GroupMessageListener::messageRenameGroup);
	registerMessage< DbmRemoveGroup >(&GroupMessageListener::messageRemoveGroup);
	registerMessage< DbmCreateGroup >(&GroupMessageListener::messageCreateGroup);
	registerMessage< DbmCreateInstance >(&GroupMessageListener::messageCreateInstance);
	registerMessage< DbmGetChildren >(&GroupMessageListener::messageGetChildren);
}

bool GroupMessageListener::messageGetGroupName(const DbmGetGroupName* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->sendReply(MsgStringResult(group->getName()));
	return true;
}

bool GroupMessageListener::messageRenameGroup(const DbmRenameGroup* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = group->rename(message->getName());
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool GroupMessageListener::messageRemoveGroup(const DbmRemoveGroup* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = group->remove();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool GroupMessageListener::messageCreateGroup(const DbmCreateGroup* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IProviderGroup > newGroup = group->createGroup(message->getName());
	if (!newGroup)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t newGroupHandle = m_connection->putObject(newGroup);
	m_connection->sendReply(MsgHandleResult(newGroupHandle));
	return true;
}

bool GroupMessageListener::messageCreateInstance(const DbmCreateInstance* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IProviderInstance > newInstance = group->createInstance(message->getName(), message->getGuid());
	if (!newInstance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t newInstanceHandle = m_connection->putObject(newInstance);
	m_connection->sendReply(MsgHandleResult(newInstanceHandle));
	return true;
}

bool GroupMessageListener::messageGetChildren(const DbmGetChildren* message)
{
	uint32_t groupHandle = message->getHandle();
	Ref< IProviderGroup > group = m_connection->getObject< IProviderGroup >(groupHandle);
	if (!group)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	RefArray< IProviderGroup > childGroup;
	RefArray< IProviderInstance > childInstances;

	if (!group->getChildren(childGroup, childInstances))
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	MsgGetChildrenResult result;
	for (RefArray< IProviderGroup >::iterator i = childGroup.begin(); i != childGroup.end(); ++i)
		result.addGroup(m_connection->putObject(*i));
	for (RefArray< IProviderInstance >::iterator i = childInstances.begin(); i != childInstances.end(); ++i)
		result.addInstance(m_connection->putObject(*i));

	m_connection->sendReply(result);
	return true;
}

	}
}
