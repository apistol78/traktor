/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "World/Entity/VolumeComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.VolumeComponentData", 0, VolumeComponentData, IEntityComponentData)

void VolumeComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void VolumeComponentData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< Aabb3, MemberAabb3 >(L"volumes", m_volumes);
}

}
