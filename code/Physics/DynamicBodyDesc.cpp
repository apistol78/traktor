#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Physics/DynamicBodyDesc.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.DynamicBodyDesc", 4, DynamicBodyDesc, BodyDesc)

DynamicBodyDesc::DynamicBodyDesc()
:	m_mass(1.0f)
,	m_autoDeactivate(true)
,	m_active(true)
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

void DynamicBodyDesc::setAutoDeactivate(bool autoDeactivate)
{
	m_autoDeactivate = autoDeactivate;
}

bool DynamicBodyDesc::getAutoDeactivate() const
{
	return m_autoDeactivate;
}

void DynamicBodyDesc::setActive(bool active)
{
	m_active = active;
}

bool DynamicBodyDesc::getActive() const
{
	return m_active;
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

bool DynamicBodyDesc::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);

	if (!BodyDesc::serialize(s))
		return false;

	s >> Member< float >(L"mass", m_mass, 0.0f);
	s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
	s >> Member< bool >(L"active", m_active);
	s >> Member< float >(L"linearDamping", m_linearDamping, 0.0f, 1.0f);
	s >> Member< float >(L"angularDamping", m_angularDamping, 0.0f, 1.0f);
	s >> Member< float >(L"friction", m_friction, 0.0f);
	
	return true;
}

	}
}
