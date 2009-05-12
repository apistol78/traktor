#include "Physics/DynamicBodyDesc.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.physics.DynamicBodyDesc", DynamicBodyDesc, BodyDesc)

DynamicBodyDesc::DynamicBodyDesc()
:	m_mass(1.0f)
,	m_autoDisable(false)
,	m_disabled(false)
,	m_linearDamping(0.0f)
,	m_angularDamping(0.0f)
,	m_friction(0.75f)
{
}

void DynamicBodyDesc::setMass(float mass)
{
	m_mass = mass;
}

float DynamicBodyDesc::getMass() const
{
	return m_mass;
}

void DynamicBodyDesc::setAutoDisable(bool autoDisable)
{
	m_autoDisable = autoDisable;
}

bool DynamicBodyDesc::getAutoDisable() const
{
	return m_autoDisable;
}

void DynamicBodyDesc::setDisabled(bool disabled)
{
	m_disabled = disabled;
}

bool DynamicBodyDesc::getDisabled() const
{
	return m_disabled;
}

void DynamicBodyDesc::setLinearDamping(float linearDamping)
{
	m_linearDamping = linearDamping;
}

float DynamicBodyDesc::getLinearDamping() const
{
	return m_linearDamping;
}

void DynamicBodyDesc::setAngularDamping(float angularDamping)
{
	m_angularDamping = angularDamping;
}

float DynamicBodyDesc::getAngularDamping() const
{
	return m_angularDamping;
}

void DynamicBodyDesc::setFriction(float friction)
{
	m_friction = friction;
}

float DynamicBodyDesc::getFriction() const
{
	return m_friction;
}

int DynamicBodyDesc::getVersion() const
{
	return 2;
}

bool DynamicBodyDesc::serialize(Serializer& s)
{
	if (!BodyDesc::serialize(s))
		return false;

	s >> Member< float >(L"mass", m_mass);
	s >> Member< bool >(L"autoDisable", m_autoDisable);
	s >> Member< bool >(L"disabled", m_disabled);

	if (s.getVersion() >= 1)
	{
		s >> Member< float >(L"linearDamping", m_linearDamping);
		s >> Member< float >(L"angularDamping", m_angularDamping);

		if (s.getVersion() >= 2)
		{
			s >> Member< float >(L"friction", m_friction);
		}
	}
	
	return true;
}

	}
}
