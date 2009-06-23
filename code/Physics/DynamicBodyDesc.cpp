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
,	m_autoDeactivate(true)
,	m_initiallyActive(true)
,	m_initiallyEnabled(true)
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

void DynamicBodyDesc::setInitiallyActive(bool initiallyActive)
{
	m_initiallyActive = initiallyActive;
}

bool DynamicBodyDesc::getInitiallyActive() const
{
	return m_initiallyActive;
}

void DynamicBodyDesc::setInitiallyEnabled(bool initiallyEnabled)
{
	m_initiallyEnabled = initiallyEnabled;
}

bool DynamicBodyDesc::getInitiallyEnabled() const
{
	return m_initiallyEnabled;
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
	return 3;
}

bool DynamicBodyDesc::serialize(Serializer& s)
{
	if (!BodyDesc::serialize(s))
		return false;

	s >> Member< float >(L"mass", m_mass, 0.0f);

	if (s.getVersion() >= 3)
	{
		s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
		s >> Member< bool >(L"initiallyActive", m_initiallyActive);
		s >> Member< bool >(L"initiallyEnabled", m_initiallyEnabled);
	}
	else
	{
		m_initiallyActive = !m_initiallyActive;
		s >> Member< bool >(L"autoDisable", m_autoDeactivate);
		s >> Member< bool >(L"disabled", m_initiallyActive);
		m_initiallyActive = !m_initiallyActive;
	}

	if (s.getVersion() >= 1)
	{
		s >> Member< float >(L"linearDamping", m_linearDamping, 0.0f, 1.0f);
		s >> Member< float >(L"angularDamping", m_angularDamping, 0.0f, 1.0f);

		if (s.getVersion() >= 2)
		{
			s >> Member< float >(L"friction", m_friction, 0.0f);
		}
	}
	
	return true;
}

	}
}
