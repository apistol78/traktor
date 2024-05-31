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
#include "Database/Remote/Messages/DbmOpen.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.DbmOpen", 0, DbmOpen, IMessage)

DbmOpen::DbmOpen(const std::wstring_view& name)
:	m_name(name)
{
}

void DbmOpen::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"name", m_name);
}

}
