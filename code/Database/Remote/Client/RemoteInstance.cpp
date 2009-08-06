#include "Database/Remote/Client/RemoteInstance.h"
#include "Database/Remote/Client/RemoteStream.h"
#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/Messages/DbmGetInstancePrimaryType.h"
#include "Database/Remote/Messages/DbmOpenTransaction.h"
#include "Database/Remote/Messages/DbmCommitTransaction.h"
#include "Database/Remote/Messages/DbmCloseTransaction.h"
#include "Database/Remote/Messages/DbmGetInstanceName.h"
#include "Database/Remote/Messages/DbmSetInstanceName.h"
#include "Database/Remote/Messages/DbmGetInstanceGuid.h"
#include "Database/Remote/Messages/DbmSetInstanceGuid.h"
#include "Database/Remote/Messages/DbmRemoveInstance.h"
#include "Database/Remote/Messages/DbmReadObject.h"
#include "Database/Remote/Messages/DbmWriteObject.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgGuidResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"
#include "Database/Remote/Messages/DbmWriteObjectResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteInstance", RemoteInstance, IProviderInstance)

RemoteInstance::RemoteInstance(Connection* connection, uint32_t handle)
:	m_connection(connection)
,	m_handle(handle)
{
}

std::wstring RemoteInstance::getPrimaryTypeName() const
{
	Ref< MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstancePrimaryType(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::openTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmOpenTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::commitTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCommitTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::closeTransaction()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmCloseTransaction(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

std::wstring RemoteInstance::getName() const
{
	Ref< MsgStringResult > result = m_connection->sendMessage< MsgStringResult >(DbmGetInstanceName(m_handle));
	return result ? result->get() : L"";
}

bool RemoteInstance::setName(const std::wstring& name)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceName(m_handle, name));
	return result ? result->getStatus() == StSuccess : false;
}

Guid RemoteInstance::getGuid() const
{
	Ref< MsgGuidResult > result = m_connection->sendMessage< MsgGuidResult >(DbmGetInstanceGuid(m_handle));
	return result ? result->get() : Guid();
}

bool RemoteInstance::setGuid(const Guid& guid)
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmSetInstanceGuid(m_handle, guid));
	return result ? result->getStatus() == StSuccess : false;
}

bool RemoteInstance::remove()
{
	Ref< MsgStatus > result = m_connection->sendMessage< MsgStatus >(DbmRemoveInstance(m_handle));
	return result ? result->getStatus() == StSuccess : false;
}

Stream* RemoteInstance::readObject(const Type*& outSerializerType)
{
	Ref< DbmReadObjectResult > result = m_connection->sendMessage< DbmReadObjectResult >(DbmReadObject(m_handle));
	if (!result)
		return 0;

	outSerializerType = Type::find(result->getSerializerTypeName());
	if (!outSerializerType)
		return 0;

	return gc_new< RemoteStream >(m_connection, result->getHandle());
}

Stream* RemoteInstance::writeObject(const std::wstring& primaryTypeName, const Type*& outSerializerType)
{
	Ref< DbmWriteObjectResult > result = m_connection->sendMessage< DbmWriteObjectResult >(DbmWriteObject(m_handle, primaryTypeName));
	if (!result)
		return 0;

	outSerializerType = Type::find(result->getSerializerTypeName());
	if (!outSerializerType)
		return 0;

	return gc_new< RemoteStream >(m_connection, result->getHandle());
}

uint32_t RemoteInstance::getDataNames(std::vector< std::wstring >& outDataNames) const
{
	return 0;
}

Stream* RemoteInstance::readData(const std::wstring& dataName)
{
	return 0;
}

Stream* RemoteInstance::writeData(const std::wstring& dataName)
{
	return 0;
}

	}
}
