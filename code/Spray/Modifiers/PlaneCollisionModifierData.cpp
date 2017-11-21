/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributeRange.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Spray/Modifiers/PlaneCollisionModifier.h"
#include "Spray/Modifiers/PlaneCollisionModifierData.h"

namespace traktor
{
	namespace spray
	{
		namespace
		{

class MemberPlane : public MemberComplex
{
public:
	MemberPlane(const wchar_t* name, Plane& value)
	:	MemberComplex(name, true)
	,	m_value(value)
	{
	}

	virtual void serialize(ISerializer& s) const
	{
		if (s.getDirection() == ISerializer::SdRead)
		{
			Vector4 normal;
			float distance;

			s >> Member< Vector4 >(L"normal", normal, AttributeDirection());
			s >> Member< float >(L"distance", distance);

			m_value.set(normal, Scalar(distance));
		}
		else	// SdWrite
		{
			Vector4 normal = m_value.normal();
			float distance = m_value.distance();

			s >> Member< Vector4 >(L"normal", normal, AttributeDirection());
			s >> Member< float >(L"distance", distance);
		}
	}

private:
	Plane& m_value;
};

		}

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PlaneCollisionModifierData", 1, PlaneCollisionModifierData, ModifierData)

PlaneCollisionModifierData::PlaneCollisionModifierData()
:	m_plane(0.0f, 1.0f, 0.0f, 0.0f)
,	m_radius(1.0f)
,	m_restitution(1.0f)
{
}

Ref< const Modifier > PlaneCollisionModifierData::createModifier(resource::IResourceManager* resourceManager) const
{
	return new PlaneCollisionModifier(m_plane, m_radius, m_restitution);
}

void PlaneCollisionModifierData::serialize(ISerializer& s)
{
	s >> MemberPlane(L"plane", m_plane);

	if (s.getVersion< PlaneCollisionModifierData >() >= 1)
		s >> Member< float >(L"radius", m_radius, AttributeRange(0.0f));

	s >> Member< float >(L"restitution", m_restitution);
}

	}
}
