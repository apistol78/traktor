/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Database/Remote/Messages/DbmWriteObject.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmWriteObject", 0, DbmWriteObject, IMessage)

DbmWriteObject::DbmWriteObject(uint32_t handle, const std::wstring& primaryTypeName)
:	m_handle(handle)
,	m_primaryTypeName(primaryTypeName)
{
}

void DbmWriteObject::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< std::wstring >(L"primaryTypeName", m_primaryTypeName);
}

	}
}
