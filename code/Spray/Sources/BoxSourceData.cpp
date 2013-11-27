#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/AttributePoint.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Spray/Sources/BoxSource.h"
#include "Spray/Sources/BoxSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.BoxSourceData", 0, BoxSourceData, SourceData)

BoxSourceData::BoxSourceData()
:	SourceData()
,	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_extent(1.0f, 1.0f, 1.0f, 0.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

Ref< const Source > BoxSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	return new BoxSource(
		getConstantRate(),
		getVelocityRate(),
		m_position,
		m_extent,
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void BoxSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> Member< Vector4 >(L"position", m_position, AttributePoint());
	s >> Member< Vector4 >(L"extent", m_extent, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

	}
}
