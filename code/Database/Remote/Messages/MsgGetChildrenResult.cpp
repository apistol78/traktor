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
#include "Database/Remote/Messages/MsgGetChildrenResult.h"

namespace traktor::db
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.db.MsgGetChildrenResult", 0, MsgGetChildrenResult, IMessage)

void MsgGetChildrenResult::addGroup(uint32_t handle)
{
	m_groupHandles.push_back(handle);
}

void MsgGetChildrenResult::addInstance(uint32_t handle)
{
	m_instanceHandles.push_back(handle);
}

const AlignedVector< uint32_t >& MsgGetChildrenResult::getGroups() const
{
	return m_groupHandles;
}

const AlignedVector< uint32_t >& MsgGetChildrenResult::getInstances() const
{
	return m_instanceHandles;
}

void MsgGetChildrenResult::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< uint32_t >(L"groupHandles", m_groupHandles);
	s >> MemberAlignedVector< uint32_t >(L"instanceHandles", m_instanceHandles);
}

}
