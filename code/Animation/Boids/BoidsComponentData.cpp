/*
 * TRAKTOR
 * Copyright (c) 2022 Anders Pistol.
 *
 * This Source Code Form is subject to the terms of the Mozilla Public
 * License, v. 2.0. If a copy of the MPL was not distributed with this
 * file, You can obtain one at https://mozilla.org/MPL/2.0/.
 */
#include "Animation/Boids/BoidsComponent.h"
#include "Animation/Boids/BoidsComponentData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "World/EntityData.h"
#include "World/IEntityBuilder.h"

namespace traktor::animation
{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.BoidsComponentData", 0, BoidsComponentData, world::IEntityComponentData)

BoidsComponentData::BoidsComponentData()
:	m_boidCount(0)
,	m_spawnPositionDiagonal(1.0f, 1.0f, 1.0f, 0.0f)
,	m_spawnVelocityDiagonal(0.0f, 0.0f, 0.0f, 0.0f)
,	m_constrain(1.0f, 1.0f, 1.0f, 1.0f)
,	m_followForce(0.0f)
,	m_repelDistance(0.0f)
,	m_repelForce(0.0f)
,	m_matchVelocityStrength(0.0f)
,	m_centerForce(0.0f)
,	m_maxVelocity(0.0f)
{
}

Ref< BoidsComponent > BoidsComponentData::createComponent(const world::IEntityBuilder* builder) const
{
	RefArray< world::Entity > boidEntities(m_boidCount);
	for (uint32_t i = 0; i < m_boidCount; ++i)
		boidEntities[i] = builder->create(m_boidEntityData);

	return new BoidsComponent(
		boidEntities,
		m_spawnPositionDiagonal,
		m_spawnVelocityDiagonal,
		m_constrain,
		m_followForce,
		m_repelDistance,
		m_repelForce,
		m_matchVelocityStrength,
		m_centerForce,
		m_maxVelocity
	);
}

void BoidsComponentData::setTransform(const world::EntityData* owner, const Transform& transform)
{
}

void BoidsComponentData::serialize(ISerializer& s)
{
	s >> MemberRef< world::EntityData >(L"boidEntityData", m_boidEntityData);
	s >> Member< uint32_t >(L"boidCount", m_boidCount);
	s >> Member< Vector4 >(L"spawnPositionDiagonal", m_spawnPositionDiagonal);
	s >> Member< Vector4 >(L"spawnVelocityDiagonal", m_spawnVelocityDiagonal);
	s >> Member< Vector4 >(L"constrain", m_constrain);
	s >> Member< float >(L"followForce", m_followForce);
	s >> Member< float >(L"repelDistance", m_repelDistance);
	s >> Member< float >(L"repelForce", m_repelForce);
	s >> Member< float >(L"matchVelocityStrength", m_matchVelocityStrength);
	s >> Member< float >(L"centerForce", m_centerForce);
	s >> Member< float >(L"maxVelocity", m_maxVelocity);
}

}
