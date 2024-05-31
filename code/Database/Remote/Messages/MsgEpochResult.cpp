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
#include "Database/Remote/Messages/MsgEpochResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgEpochResult", 0, MsgEpochResult, IMessage)

MsgEpochResult::MsgEpochResult(uint64_t value)
:	m_value(value)
{
}

void MsgEpochResult::serialize(ISerializer& s)
{
	s >> Member< uint64_t >(L"value", m_value);
}

}
