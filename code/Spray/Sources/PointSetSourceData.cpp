#include "Core/Math/Const.h"
#include "Core/Serialization/AttributeDirection.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"
#include "Resource/IResourceManager.h"
#include "Resource/Member.h"
#include "Spray/PointSet.h"
#include "Spray/Sources/PointSetSource.h"
#include "Spray/Sources/PointSetSourceData.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.PointSetSourceData", 0, PointSetSourceData, SourceData)

PointSetSourceData::PointSetSourceData()
:	SourceData()
,	m_offset(0.0f, 0.0f, 0.0f, 0.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

Ref< const Source > PointSetSourceData::createSource(resource::IResourceManager* resourceManager) const
{
	resource::Proxy< PointSet > pointSet;
	if (!resourceManager->bind(m_pointSet, pointSet))
		return 0;

	return new PointSetSource(
		getConstantRate(),
		getVelocityRate(),
		pointSet,
		m_offset,
		m_velocity,
		m_orientation,
		m_angularVelocity,
		m_age,
		m_mass,
		m_size
	);
}

void PointSetSourceData::serialize(ISerializer& s)
{
	SourceData::serialize(s);

	s >> resource::Member< PointSet >(L"pointSet", m_pointSet);
	s >> Member< Vector4 >(L"offset", m_offset, AttributeDirection());
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);
}

	}
}
