/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/DbmGetGroupName.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetGroupName", 0, DbmGetGroupName, IMessage)

DbmGetGroupName::DbmGetGroupName(uint32_t handle)
:	m_handle(handle)
{
}

void DbmGetGroupName::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

	}
}
