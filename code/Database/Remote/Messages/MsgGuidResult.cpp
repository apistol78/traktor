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
#include "Database/Remote/Messages/MsgGuidResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgGuidResult", 0, MsgGuidResult, IMessage)

MsgGuidResult::MsgGuidResult(const Guid& value)
:	m_value(value)
{
}

void MsgGuidResult::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"value", m_value);
}

}
