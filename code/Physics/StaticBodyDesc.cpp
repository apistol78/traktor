#include "Physics/StaticBodyDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.StaticBodyDesc", StaticBodyDesc, BodyDesc)

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

int StaticBodyDesc::getVersion() const
{
	return 2;
}

bool StaticBodyDesc::serialize(Serializer& s)
{
	if (!BodyDesc::serialize(s))
		return false;

	if (s.getVersion() >= 1)
	{
		s >> Member< float >(L"friction", m_friction);
		if (s.getVersion() >= 2)
		{
			s >> Member< bool >(L"kinematic", m_kinematic);
		}
	}
	
	return true;
}

	}
}
