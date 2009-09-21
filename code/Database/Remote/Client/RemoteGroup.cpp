#include "Database/Remote/Client/RemoteGroup.h"
#include "Database/Remote/Client/RemoteInstance.h"
#include "Database/Remote/Client/Connection.h"
#include "Database/Remote/Messages/CnmReleaseObject.h"
#include "Database/Remote/Messages/DbmGetGroupName.h"
#include "Database/Remote/Messages/DbmRenameGroup.h"
#include "Database/Remote/Messages/DbmRemoveGroup.h"
#include "Database/Remote/Messages/DbmCreateGroup.h"
#include "Database/Remote/Messages/DbmCreateInstance.h"
#include "Database/Remote/Messages/DbmGetChildGroups.h"
#include "Database/Remote/Messages/DbmGetChildInstances.h"
#include "Database/Remote/Messages/MsgStringResult.h"
#include "Database/Remote/Messages/MsgHandleResult.h"
#include "Database/Remote/Messages/MsgHandleArrayResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.db.RemoteGroup", RemoteGroup, IProviderGroup)

RemoteGroup::RemoteGroup(Connection* connection, uint32_t handle)
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

IProviderGroup* RemoteGroup::createGroup(const std::wstring& groupName)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateGroup(m_handle, groupName));
	return result ? gc_new< RemoteGroup >(m_connection, result->get()) : 0;
}

IProviderInstance* RemoteGroup::createInstance(const std::wstring& instanceName, const Guid& instanceGuid)
{
	Ref< MsgHandleResult > result = m_connection->sendMessage< MsgHandleResult >(DbmCreateInstance(m_handle, instanceName, instanceGuid));
	return result ? gc_new< RemoteInstance >(m_connection, result->get()) : 0;
}

bool RemoteGroup::getChildGroups(RefArray< IProviderGroup >& outChildGroups)
{
	Ref< MsgHandleArrayResult > result = m_connection->sendMessage< MsgHandleArrayResult >(DbmGetChildGroups(m_handle));
	if (!result)
		return false;

	for (uint32_t i = 0; i < result->count(); ++i)
		outChildGroups.push_back(gc_new< RemoteGroup >(m_connection, result->get(i)));

	return true;
}

bool RemoteGroup::getChildInstances(RefArray< IProviderInstance >& outChildInstances)
{
	Ref< MsgHandleArrayResult > result = m_connection->sendMessage< MsgHandleArrayResult >(DbmGetChildInstances(m_handle));
	if (!result)
		return false;

	for (uint32_t i = 0; i < result->count(); ++i)
		outChildInstances.push_back(gc_new< RemoteInstance >(m_connection, result->get(i)));

	return true;
}

	}
}
