/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Remote/Messages/DbmWriteObjectResult.h"

namespace traktor
{
	namespace db
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmWriteObjectResult", 0, DbmWriteObjectResult, IMessage)

DbmWriteObjectResult::DbmWriteObjectResult(uint32_t streamId, const std::wstring& serializerTypeName)
:	m_streamId(streamId)
,	m_serializerTypeName(serializerTypeName)
{
}

void DbmWriteObjectResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"streamId", m_streamId);
	s >> Member< std::wstring >(L"serializerTypeName", m_serializerTypeName);
}

	}
}
