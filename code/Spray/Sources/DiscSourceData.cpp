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
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/DiscSource.h"
#include "Spray/Sources/DiscSourceData.h"

namespace traktor::spray
{
	namespace
	{

const resource::Id< render::Shader > c_shaderDiscSource(L"{83FA30DA-8CAA-5A42-A807-6BF398DE52FA}");

	}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DiscSourceData", 0, DiscSourceData, SourceData)

DiscSourceData::DiscSourceData()
:	SourceData()
,	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_normal(0.0f, 1.0f, 0.0f, 0.0f)
,	m_radius(1.0f, 2.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

resource::Id< render::Shader > DiscSourceData::getShader() const
{
	return c_shaderDiscSource;
}

Ref< const Source > DiscSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new DiscSource(
		getConstantRate(),
		getVelocityRate(),
		m_position,
		m_normal,
		m_radius,
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void DiscSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> Member< Vector4 >(L"position", m_position, AttributePoint());
	s >> Member< Vector4 >(L"normal", m_normal, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"radius", m_radius);
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

}
