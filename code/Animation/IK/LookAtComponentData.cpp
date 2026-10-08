/*
 * TRAKTOR
 * Copyright (c) 2026 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/IK/LookAtComponentData.h"

#include "Animation/IK/LookAtComponent.h"
#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.LookAtComponentData", 1, LookAtComponentData, world::IEntityComponentData)

Ref< LookAtComponent > LookAtComponentData::createComponent() const
{
	LookAtComponent::Settings settings;
	settings.joint = !m_joint.empty() ? render::getParameterHandle(m_joint) : 0;
	settings.eyeOffset = m_eyeOffset.xyz0();
	settings.maxYaw = deg2rad(m_maxYaw);
	settings.maxPitch = deg2rad(m_maxPitch);
	settings.smoothTime = m_smoothTime;
	settings.cullDistance = m_cullDistance;
	return new LookAtComponent(settings);
}

int32_t LookAtComponentData::getOrdinal() const
{
	return -40;
}

void LookAtComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void LookAtComponentData::serialize(ISerializer& s)
{
	s >> Member< std::wstring >(L"joint", m_joint);
	if (s.getVersion() >= 1)
		s >> Member< Vector4 >(L"eyeOffset", m_eyeOffset, AttributeUnit(UnitType::Metres));
	s >> Member< float >(L"maxYaw", m_maxYaw, AttributeUnit(UnitType::Degrees) | AttributeRange(0.0f, 180.0f));
	s >> Member< float >(L"maxPitch", m_maxPitch, AttributeUnit(UnitType::Degrees) | AttributeRange(0.0f, 90.0f));
	s >> Member< float >(L"smoothTime", m_smoothTime, AttributeUnit(UnitType::Seconds) | AttributeRange(0.0f));
	s >> Member< float >(L"cullDistance", m_cullDistance, AttributeUnit(UnitType::Metres) | AttributeRange(0.0f));
}

}
