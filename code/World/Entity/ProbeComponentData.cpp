/*
 * TRAKTOR
 * Copyright (c) 2022-2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "World/Entity/ProbeComponentData.h"

#include "Core/Math/Aabb3.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Render/ITexture.h"
#include "Resource/Member.h"

namespace traktor::world
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ProbeComponentData", 1, ProbeComponentData, IEntityComponentData)

int32_t ProbeComponentData::getOrdinal() const
{
	return 0;
}

void ProbeComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void ProbeComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ITexture >(L"texture", m_texture);
	s >> Member< float >(L"intensity", m_intensity, AttributeUnit(UnitType::Percent));

	if (s.getVersion< ProbeComponentData >() < 1)
	{
		Aabb3 volume;
		s >> ObsoleteMember< bool >(L"local");
		s >> MemberAabb3(L"volume", volume);
	}
}

}
