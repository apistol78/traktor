/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/DbmSetInstanceGuid.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmSetInstanceGuid", 0, DbmSetInstanceGuid, IMessage)

DbmSetInstanceGuid::DbmSetInstanceGuid(uint32_t handle, const Guid& guid)
:	m_handle(handle)
,	m_guid(guid)
{
}

void DbmSetInstanceGuid::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< Guid >(L"guid", m_guid);
}

	}
}
