/*
 * TRAKTOR
 * Copyright (c) 2023 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "World/Entity/OccluderComponent.h"
#include "World/Entity/OccluderComponentData.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.OccluderComponentData", 2, OccluderComponentData, IEntityComponentData)

Ref< OccluderComponent > OccluderComponentData::createComponent() const
{
	return new OccluderComponent(m_volume);
}

int32_t OccluderComponentData::getOrdinal() const
{
	return 0;
}

void OccluderComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void OccluderComponentData::serialize(ISerializer& s)
{
    s >> MemberAabb3(L"volume", m_volume);
}

}
