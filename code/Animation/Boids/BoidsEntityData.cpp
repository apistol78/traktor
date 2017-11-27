/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Animation/Boids/BoidsEntity.h"
#include "Animation/Boids/BoidsEntityData.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberRef.h"
#include "World/IEntityBuilder.h"

namespace traktor
{
	namespace animation
	{

T_IMPLEMENT_RTTI_EDIT_CLASS(L"traktor.animation.BoidsEntityData", 1, BoidsEntityData, world::EntityData)

BoidsEntityData::BoidsEntityData()
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

Ref< BoidsEntity > BoidsEntityData::createEntity(const world::IEntityBuilder* builder) const
{
	RefArray< world::Entity > boidEntities(m_boidCount);
	for (uint32_t i = 0; i < m_boidCount; ++i)
		boidEntities[i] = builder->create(m_boidEntityData);
	return new BoidsEntity(
		boidEntities,
		getTransform(),
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

void BoidsEntityData::serialize(ISerializer& s)
{
	world::EntityData::serialize(s);

	s >> MemberRef< world::EntityData >(L"boidEntityData", m_boidEntityData);
	s >> Member< uint32_t >(L"boidCount", m_boidCount);
	s >> Member< Vector4 >(L"spawnPositionDiagonal", m_spawnPositionDiagonal);
	s >> Member< Vector4 >(L"spawnVelocityDiagonal", m_spawnVelocityDiagonal);

	if (s.getVersion() >= 1)
		s >> Member< Vector4 >(L"constrain", m_constrain);

	s >> Member< float >(L"followForce", m_followForce);
	s >> Member< float >(L"repelDistance", m_repelDistance);
	s >> Member< float >(L"repelForce", m_repelForce);
	s >> Member< float >(L"matchVelocityStrength", m_matchVelocityStrength);
	s >> Member< float >(L"centerForce", m_centerForce);
	s >> Member< float >(L"maxVelocity", m_maxVelocity);
}

	}
}
