/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Remote/Messages/DbmReadObjectResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmReadObjectResult", 0, DbmReadObjectResult, IMessage)

DbmReadObjectResult::DbmReadObjectResult(uint32_t streamId, const std::wstring_view& serializerTypeName)
:	m_streamId(streamId)
,	m_serializerTypeName(serializerTypeName)
{
}

void DbmReadObjectResult::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"streamId", m_streamId);
	s >> Member< std::wstring >(L"serializerTypeName", m_serializerTypeName);
}

}
