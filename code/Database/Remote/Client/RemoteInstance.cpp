/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Io/BufferedStream.h"
#include "Database/Types.h"
#include "Database/Remote/Client/RemoteInstance.h"
#include "Database/Remote/Client/RemoteConnection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
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
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgStringArrayResult.h"
#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"
#include "Database/Remote/Messages/DbmWriteObjectResult.h"
#include "Net/Stream/RemoteStream.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteInstance", RemoteInstance, IProviderInstance)

RemoteInstance::RemoteInstance(RemoteConnection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

RemoteInstance::~RemoteInstance()
{
	if (m_connection)
		m_connection->sendMessage< MsgStatus >(CnmReleaseObject(m_handle));
}

std::wstring RemoteInstance::getPrimaryTypeName() const
{
	Ref< const MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstancePrimaryType(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::openTransaction()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmOpenTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::commitTransaction()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCommitTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::closeTransaction()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCloseTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

std::wstring RemoteInstance::getName() const
{
	Ref< const MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstanceName(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::setName(const std::wstring& name)
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceName(m_handle, name));
	return result ? result->getStatus() == StSuccess : false;
}

Guid RemoteInstance::getGuid() const
{
	Ref< const MsgGuidResult > result = m_connection->sendMessage< MsgGuidResult >(DbmGetInstanceGuid(m_handle));
	return result ? result->get() : Guid();
}

bool RemoteInstance::setGuid(const Guid& guid)
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceGuid(m_handle, guid));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::getLastModifyDate(DateTime& outModifyDate) const
{
	return false;
}

uint32_t RemoteInstance::getFlags() const
{
	return IfNormal;
}

bool RemoteInstance::remove()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveInstance(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Ref< IStream > RemoteInstance::readObject(const TypeInfo*& outSerializerType) const
{
	Ref< const DbmReadObjectResult > result = m_connection->sendMessage< DbmReadObjectResult >(DbmReadObject(m_handle));
	if (!result)
		return nullptr;

	outSerializerType = TypeInfo::find(result->getSerializerTypeName().c_str());
	if (!outSerializerType)
		return nullptr;

	Ref< IStream > s = net::RemoteStream::connect(m_connection->getStreamServerAddr(), result->getStreamId());
	if (!s)
		return nullptr;

	return BufferedStream::createIfNotAlready(s);
}

Ref< IStream > RemoteInstance::writeObject(const std::wstring& primaryTypeName, const TypeInfo*& outSerializerType)
{
	Ref< const DbmWriteObjectResult > result = m_connection->sendMessage< DbmWriteObjectResult >(DbmWriteObject(m_handle, primaryTypeName));
	if (!result)
		return nullptr;

	outSerializerType = TypeInfo::find(result->getSerializerTypeName().c_str());
	if (!outSerializerType)
		return nullptr;

	Ref< IStream > s = net::RemoteStream::connect(m_connection->getStreamServerAddr(), result->getStreamId());
	if (!s)
		return nullptr;

	return BufferedStream::createIfNotAlready(s);
}

uint32_t RemoteInstance::getDataNames(AlignedVector< std::wstring >& outDataNames) const
{
	Ref< const MsgStringArrayResult > result = m_connection->sendMessage< MsgStringArrayResult >(DbmGetDataNames(m_handle));
	if (!result)
		return 0;

	outDataNames = result->get();
	return (uint32_t)outDataNames.size();
}

bool RemoteInstance::getDataLastWriteTime(const std::wstring& dataName, DateTime& outLastWriteTime) const
{
	return false;
}

bool RemoteInstance::removeAllData()
{
	Ref< const MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveAllData(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Ref< IStream > RemoteInstance::readData(const std::wstring& dataName) const
{
	Ref< const MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmReadData(m_handle, dataName));
	if (!result)
		return nullptr;

	Ref< IStream > s = net::RemoteStream::connect(m_connection->getStreamServerAddr(), result->get());
	if (!s)
		return nullptr;

	return BufferedStream::createIfNotAlready(s);
}

Ref< IStream > RemoteInstance::writeData(const std::wstring& dataName)
{
	Ref< const MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmWriteData(m_handle, dataName));
	if (!result)
		return nullptr;

	Ref< IStream > s = net::RemoteStream::connect(m_connection->getStreamServerAddr(), result->get());
	if (!s)
		return nullptr;

	return BufferedStream::createIfNotAlready(s);
}

}
