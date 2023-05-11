/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributePrivate.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "World/Entity/PersistentIdComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.PersistentIdComponentData", 0, PersistentIdComponentData, IEntityComponentData)

PersistentIdComponentData::PersistentIdComponentData()
{
	m_id = Guid::create();
}

const Guid& PersistentIdComponentData::getId() const
{
	return m_id;
}

int32_t PersistentIdComponentData::getOrdinal() const
{
	return 0;
}

void PersistentIdComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void PersistentIdComponentData::serialize(ISerializer& s)
{
	s >> Member< Guid >(L"id", m_id, AttributePrivate());
}

}
