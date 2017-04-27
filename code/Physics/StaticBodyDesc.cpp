/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/StaticBodyDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.StaticBodyDesc", 4, StaticBodyDesc, BodyDesc)

StaticBodyDesc::StaticBodyDesc()
:	m_friction(0.75f)
,	m_kinematic(false)
{
}

void StaticBodyDesc::setFriction(float friction)
{
	m_friction = friction;
}

float StaticBodyDesc::getFriction() const
{
	return m_friction;
}

void StaticBodyDesc::setKinematic(bool kinematic)
{
	m_kinematic = kinematic;
}

bool StaticBodyDesc::isKinematic() const
{
	return m_kinematic;
}

void StaticBodyDesc::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);

	BodyDesc::serialize(s);

	s >> Member< float >(L"friction", m_friction, AttributeRange(0.0f, 1.0f));
	s >> Member< bool >(L"kinematic", m_kinematic);
}

	}
}
