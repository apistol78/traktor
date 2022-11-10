/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAabb.h"
#include "Render/ICubeTexture.h"
#include "Resource/Member.h"
#include "World/Entity/ProbeComponentData.h"

namespace traktor
{
	namespace world
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.world.ProbeComponentData", 0, ProbeComponentData, IEntityComponentData)

void ProbeComponentData::setTransform(const EntityData* owner, const Transform& transform)
{
}

void ProbeComponentData::serialize(ISerializer& s)
{
	s >> resource::Member< render::ICubeTexture >(L"texture", m_texture);
	s >> Member< float >(L"intensity", m_intensity, AttributeUnit(UnitType::Percent));
	s >> Member< bool >(L"local", m_local);
	s >> MemberAabb3(L"volume", m_volume);
}

	}
}
