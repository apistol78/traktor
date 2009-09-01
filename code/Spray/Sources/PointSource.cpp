#include "Spray/Sources/PointSource.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/EmitterInstance.h"
#include "Core/Serialization/Serializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_SERIALIZABLE_CLASS(L"traktor.spray.PointSource", PointSource, Source)

PointSource::PointSource()
:	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

void PointSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	Vector4 position = transform * m_position;

	while (emitCount-- > 0)
	{
		Point point;

		point.position = position;
		point.velocity = context.random.nextUnit() * Scalar(m_velocity.random(context.random));
		point.orientation = m_orientation.random(context.random);
		point.angularVelocity = m_angularVelocity.random(context.random);
		point.age = 0.0f;
		point.maxAge = m_age.random(context.random);
		point.inverseMass = 1.0f / (m_mass.random(context.random));
		point.size = m_size.random(context.random);
		point.random = context.random.nextFloat();

		emitterInstance.addPoint(point);
	}
}

bool PointSource::serialize(Serializer& s)
{
	if (!Source::serialize(s))
		return false;

	s >> Member< Vector4 >(L"position", m_position);
	s >> MemberComposite< Range< float > >(L"velocity", m_velocity);
	s >> MemberComposite< Range< float > >(L"orientation", m_orientation);
	s >> MemberComposite< Range< float > >(L"angularVelocity", m_angularVelocity);
	s >> MemberComposite< Range< float > >(L"age", m_age);
	s >> MemberComposite< Range< float > >(L"mass", m_mass);
	s >> MemberComposite< Range< float > >(L"size", m_size);

	return true;
}

	}
}
