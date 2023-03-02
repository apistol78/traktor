/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Database/Remote/Messages/DbmGetEvent.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmGetEvent", 0, DbmGetEvent, IMessage)

DbmGetEvent::DbmGetEvent(uint32_t handle, uint64_t sqnr)
:	m_handle(handle)
,	m_sqnr(sqnr)
{
}

void DbmGetEvent::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
	s >> Member< uint64_t >(L"m_sqnr", m_sqnr);
}

}
