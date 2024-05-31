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
#include "Database/Remote/Messages/MsgStatus.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgStatus", 0, MsgStatus, IMessage)

MsgStatus::MsgStatus(MsgStatusType status)
:	m_status(int32_t(status))
{
}

void MsgStatus::serialize(ISerializer& s)
{
	s >> Member< int32_t >(L"status", m_status);
}

}
