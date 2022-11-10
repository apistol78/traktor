/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Rotator/WobbleComponentData.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.WobbleComponentData", 0, WobbleComponentData, world::IEntityComponentData)

Ref< WobbleComponent > WobbleComponentData::createComponent() const
{
	return new WobbleComponent(m_magnitude, m_rate);
}

void WobbleComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void WobbleComponentData::serialize(ISerializer& s)
{
	s >> Member< float >(L"magnitude", m_magnitude, AttributeRange(0.0f));
	s >> Member< float >(L"rate", m_rate, AttributeRange(0.0f));
}

	}
}
