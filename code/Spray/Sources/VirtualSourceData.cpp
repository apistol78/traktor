/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Math/Const.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/VirtualSource.h"
#include "Spray/Sources/VirtualSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.VirtualSourceData", 0, VirtualSourceData, SourceData)

VirtualSourceData::VirtualSourceData()
:	SourceData()
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

Ref< const Source > VirtualSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new VirtualSource(
		getConstantRate(),
		getVelocityRate(),
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void VirtualSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

	}
}
