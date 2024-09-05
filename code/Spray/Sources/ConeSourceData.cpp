/*
 * TRAKTOR
 * Copyright (c) 2022-2024 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/AttributeUnit.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/ConeSource.h"
#include "Spray/Sources/ConeSourceData.h"

namespace traktor::spray
{
	namespace
	{

const resource::Id< render::Shader > c_shaderConeSource(L"{1BF7210A-0A23-E041-988D-44AADC38D06E}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.ConeSourceData", 1, ConeSourceData, SourceData)

ConeSourceData::ConeSourceData()
:	SourceData()
,	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_normal(0.0f, 1.0f, 0.0f, 0.0f)
,	m_angle1(HALF_PI)
,	m_angle2(HALF_PI)
,	m_velocity(0.0f, 0.0f)
,	m_inheritVelocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

resource::Id< render::Shader > ConeSourceData::getShader() const
{
	return c_shaderConeSource;
}

Ref< const Source > ConeSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new ConeSource(
		getConstantRate(),
		getVelocityRate(),
		m_position,
		m_normal,
		m_angle1,
		m_angle2,
		m_velocity,
		m_inheritVelocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void ConeSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> Member< Vector4 >(L"position", m_position, AttributePoint());
	s >> Member< Vector4 >(L"normal", m_normal, AttributeDirection());
	s >> Member< float >(L"angle1", m_angle1, AttributeUnit(UnitType::Radians) | AttributeRange(0.0f, HALF_PI));
	s >> Member< float >(L"angle2", m_angle2, AttributeUnit(UnitType::Radians) | AttributeRange(0.0f, HALF_PI));
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);

	if (s.getVersion() >= 1)
		s >> MemberComposite< Range< float > >(L"inheritVelocity", m_inheritVelocity);

	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

}
