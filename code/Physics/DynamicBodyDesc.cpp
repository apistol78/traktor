#include "Physics/DynamicBodyDesc.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"

namespace traktor
{
	namespace physics
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.physics.DynamicBodyDesc", 5, DynamicBodyDesc, BodyDesc)

DynamicBodyDesc::DynamicBodyDesc()
:	m_mass(1.0f)
,	m_autoDeactivate(true)
,	m_active(true)
,	m_linearDamping(0.0f)
,	m_angularDamping(0.0f)
,	m_friction(0.75f)
,	m_linearThreshold(0.8f)
,	m_angularThreshold(1.0f)
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

void DynamicBodyDesc::setLinearThreshold(float linearThreshold)
{
	m_linearThreshold = linearThreshold;
}

float DynamicBodyDesc::getLinearThreshold() const
{
	return m_linearThreshold;
}

void DynamicBodyDesc::setAngularThreshold(float angularThreshold)
{
	m_angularThreshold = angularThreshold;
}

float DynamicBodyDesc::getAngularThreshold() const
{
	return m_angularThreshold;
}

void DynamicBodyDesc::serialize(ISerializer& s)
{
	T_ASSERT (s.getVersion() >= 4);

	BodyDesc::serialize(s);

	s >> Member< float >(L"mass", m_mass, AttributeRange(0.0f));
	s >> Member< bool >(L"autoDeactivate", m_autoDeactivate);
	s >> Member< bool >(L"active", m_active);
	s >> Member< float >(L"linearDamping", m_linearDamping, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"angularDamping", m_angularDamping, AttributeRange(0.0f, 1.0f));
	s >> Member< float >(L"friction", m_friction, AttributeRange(0.0f));

	if (s.getVersion() >= 5)
	{
		s >> Member< float >(L"linearThreshold", m_linearThreshold, AttributeRange(0.0f));
		s >> Member< float >(L"angularThreshold", m_angularThreshold, AttributeRange(0.0f));
	}
}

	}
}
