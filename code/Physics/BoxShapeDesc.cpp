/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Physics/BoxShapeDesc.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.BoxShapeDesc", ShapeDesc::Version, BoxShapeDesc, ShapeDesc)

BoxShapeDesc::BoxShapeDesc()
:	m_extent(0.0f, 0.0f, 0.0f, 0.0f)
,	m_margin(0.0f)
{
}

void BoxShapeDesc::setExtent(const Vector4& extent)
{
	m_extent = extent;
}

const Vector4& BoxShapeDesc::getExtent() const
{
	return m_extent;
}

void BoxShapeDesc::setMargin(float margin)
{
	m_margin = margin;
}

float BoxShapeDesc::getMargin() const
{
	return m_margin;
}

void BoxShapeDesc::serialize(ISerializer& s)
{
	ShapeDesc::serialize(s);
	s >> Member< Vector4 >(L"extent", m_extent, AttributeDirection());
	if (s.getVersion() >= 6)
		s >> Member< float >(L"margin", m_margin, AttributeRange(0.0f) | AttributeUnit(UnitType::Metres));
}

	}
}
