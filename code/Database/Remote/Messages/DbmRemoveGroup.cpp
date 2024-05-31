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
#include "Database/Remote/Messages/DbmRemoveGroup.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmRemoveGroup", 0, DbmRemoveGroup, IMessage)

DbmRemoveGroup::DbmRemoveGroup(uint32_t handle)
:	m_handle(handle)
{
}

void DbmRemoveGroup::serialize(ISerializer& s)
{
	s >> Member< uint32_t >(L"handle", m_handle);
}

}
