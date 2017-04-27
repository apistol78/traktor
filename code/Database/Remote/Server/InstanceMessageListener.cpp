/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Io/IStream.h"
#include "Database/Provider/IProviderInstance.h"
#include "Database/Remote/Messages/DbmGetInstancePrimaryType.h"
#include "Database/Remote/Messages/DbmOpenTransaction.h"
#include "Database/Remote/Messages/DbmCommitTransaction.h"
#include "Database/Remote/Messages/DbmCloseTransaction.h"
#include "Database/Remote/Messages/DbmGetInstanceName.h"
#include "Database/Remote/Messages/DbmSetInstanceName.h"
#include "Database/Remote/Messages/DbmGetInstanceGuid.h"
#include "Database/Remote/Messages/DbmSetInstanceGuid.h"
#include "Database/Remote/Messages/DbmRemoveAllData.h"
#include "Database/Remote/Messages/DbmRemoveInstance.h"
#include "Database/Remote/Messages/DbmReadObject.h"
#include "Database/Remote/Messages/DbmWriteObject.h"
#include "Database/Remote/Messages/DbmGetDataNames.h"
#include "Database/Remote/Messages/DbmReadData.h"
#include "Database/Remote/Messages/DbmWriteData.h"
#include "Database/Remote/Messages/MsgStatus.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgStringArrayResult.h"
#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"
#include "Database/Remote/Messages/DbmWriteObjectResult.h"
#include "Database/Remote/Server/Connection.h"
#include "Database/Remote/Server/InstanceMessageListener.h"
#include "Net/Stream/StreamServer.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.InstanceMessageListener", InstanceMessageListener, IMessageListener)

InstanceMessageListener::InstanceMessageListener(Connection* connection)
:	m_connection(connection)
{
	registerMessage< DbmGetInstancePrimaryType >(&InstanceMessageListener::messageGetInstancePrimaryType);
	registerMessage< DbmOpenTransaction >(&InstanceMessageListener::messageOpenTransaction);
	registerMessage< DbmCommitTransaction >(&InstanceMessageListener::messageCommitTransaction);
	registerMessage< DbmCloseTransaction >(&InstanceMessageListener::messageCloseTransaction);
	registerMessage< DbmGetInstanceName >(&InstanceMessageListener::messageGetInstanceName);
	registerMessage< DbmSetInstanceName >(&InstanceMessageListener::messageSetInstanceName);
	registerMessage< DbmGetInstanceGuid >(&InstanceMessageListener::messageGetInstanceGuid);
	registerMessage< DbmSetInstanceGuid >(&InstanceMessageListener::messageSetInstanceGuid);
	registerMessage< DbmRemoveInstance >(&InstanceMessageListener::messageRemoveInstance);
	registerMessage< DbmReadObject >(&InstanceMessageListener::messageReadObject);
	registerMessage< DbmWriteObject >(&InstanceMessageListener::messageWriteObject);
	registerMessage< DbmGetDataNames >(&InstanceMessageListener::messageGetDataNames);
	registerMessage< DbmRemoveAllData >(&InstanceMessageListener::messageRemoveAllData);
	registerMessage< DbmReadData >(&InstanceMessageListener::messageReadData);
	registerMessage< DbmWriteData >(&InstanceMessageListener::messageWriteData);
}

bool InstanceMessageListener::messageGetInstancePrimaryType(const DbmGetInstancePrimaryType* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->sendReply(MsgStringResult(instance->getPrimaryTypeName()));
	return true;
}

bool InstanceMessageListener::messageOpenTransaction(const DbmOpenTransaction* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->openTransaction();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageCommitTransaction(const DbmCommitTransaction* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->commitTransaction();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageCloseTransaction(const DbmCloseTransaction* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->closeTransaction();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageGetInstanceName(const DbmGetInstanceName* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	m_connection->sendReply(MsgStringResult(instance->getName()));
	return true;
}

bool InstanceMessageListener::messageSetInstanceName(const DbmSetInstanceName* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->setName(message->getName());
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageGetInstanceGuid(const DbmGetInstanceGuid* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Guid guid = instance->getGuid();
	m_connection->sendReply(MsgGuidResult(guid));
	return true;
}

bool InstanceMessageListener::messageSetInstanceGuid(const DbmSetInstanceGuid* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->setGuid(message->getGuid());
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageRemoveInstance(const DbmRemoveInstance* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->remove();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageReadObject(const DbmReadObject* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	const TypeInfo* serializerType = 0;
	Ref< IStream > objectStream = instance->readObject(serializerType);
	if (!objectStream || !serializerType)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t objectStreamId = m_connection->getStreamServer()->publish(objectStream);
	m_connection->sendReply(DbmReadObjectResult(objectStreamId, serializerType->getName()));
	return true;
}

bool InstanceMessageListener::messageWriteObject(const DbmWriteObject* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	const TypeInfo* serializerType = 0;
	Ref< IStream > objectStream = instance->writeObject(message->getPrimaryTypeName(), serializerType);
	if (!objectStream || !serializerType)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t objectStreamId = m_connection->getStreamServer()->publish(objectStream);
	m_connection->sendReply(DbmWriteObjectResult(objectStreamId, serializerType->getName()));
	return true;
}

bool InstanceMessageListener::messageGetDataNames(const DbmGetDataNames* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	std::vector< std::wstring > dataNames;
	instance->getDataNames(dataNames);

	m_connection->sendReply(MsgStringArrayResult(dataNames));
	return true;
}

bool InstanceMessageListener::messageRemoveAllData(const DbmRemoveAllData* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	bool result = instance->removeAllData();
	m_connection->sendReply(MsgStatus(result ? StSuccess : StFailure));
	return true;
}

bool InstanceMessageListener::messageReadData(const DbmReadData* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IStream > dataStream = instance->readData(message->getName());
	if (!dataStream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t dataStreamId = m_connection->getStreamServer()->publish(dataStream);
	m_connection->sendReply(MsgHandleResult(dataStreamId));
	return true;
}

bool InstanceMessageListener::messageWriteData(const DbmWriteData* message)
{
	uint32_t instanceHandle = message->getHandle();
	Ref< IProviderInstance > instance = m_connection->getObject< IProviderInstance >(instanceHandle);
	if (!instance)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	Ref< IStream > dataStream = instance->writeData(message->getName());
	if (!dataStream)
	{
		m_connection->sendReply(MsgStatus(StFailure));
		return true;
	}

	uint32_t dataStreamId = m_connection->getStreamServer()->publish(dataStream);
	m_connection->sendReply(MsgHandleResult(dataStreamId));
	return true;
}

	}
}
