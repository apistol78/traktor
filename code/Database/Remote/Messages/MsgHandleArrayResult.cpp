/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Database/Remote/Messages/MsgHandleArrayResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgHandleArrayResult", 0, MsgHandleArrayResult, IMessage)

void MsgHandleArrayResult::add(uint32_t handle)
{
	m_handles.push_back(handle);
}

uint32_t MsgHandleArrayResult::count()
{
	return uint32_t(m_handles.size());
}

uint32_t MsgHandleArrayResult::get(uint32_t index) const
{
	return m_handles[index];
}

void MsgHandleArrayResult::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< uint32_t >(L"handles", m_handles);
}

}
