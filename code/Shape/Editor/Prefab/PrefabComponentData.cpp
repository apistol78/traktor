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
#include "Shape/Editor/Prefab/PrefabComponentData.h"

namespace traktor::shape
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.shape.PrefabComponentData", 1, PrefabComponentData, world::IEntityComponentData)

PrefabComponentData::PrefabComponentData()
:	m_id(Guid::create())
{
}

int32_t PrefabComponentData::getOrdinal() const
{
	return -100000;
}

void PrefabComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void PrefabComponentData::serialize(ISerializer& s)
{
	if (s.getVersion< PrefabComponentData >() >= 1)
		s >> Member< Guid >(L"id", m_id, AttributePrivate());
}

}
