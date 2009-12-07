#include "Spray/Sources/DiscSource.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/EmitterInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.DiscSource", 0, DiscSource, Source)

DiscSource::DiscSource()
:	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_normal(0.0f, 1.0f, 0.0f, 0.0f)
,	m_radius(1.0f, 2.0f)
,	m_velocity(0.0f, 0.0f)
,	m_orientation(0.0f, 2.0f * PI)
,	m_angularVelocity(0.0f, 0.0f)
,	m_age(1.0f, 1.0f)
,	m_mass(1.0f, 1.0f)
,	m_size(1.0f, 1.0f)
{
}

bool DiscSource::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

void DiscSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	const Vector4 axisZ(0.0f, 0.0f, 1.0f, 0.0f);

	Vector4 wx = cross(m_normal, axisZ);
	Vector4 wz = cross(wx, m_normal);

	Vector4 position = transform * m_position;
	Vector4 x = transform * wx;
	Vector4 y = transform * m_normal;
	Vector4 z = transform * wz;

	while (emitCount-- > 0)
	{
		Vector4 direction = (x * Scalar(context.random.nextFloat() * 2.0f - 1.0f) + z * Scalar(context.random.nextFloat() * 2.0f - 1.0f)).normalized();

		Point point;

		point.position = position + direction * Scalar(m_radius.random(context.random));
		point.velocity = y * Scalar(m_velocity.random(context.random));
		point.orientation = m_orientation.random(context.random);
		point.angularVelocity = m_angularVelocity.random(context.random);
		point.color = Vector4::one();
		point.age = 0.0f;
		point.maxAge = m_age.random(context.random);
		point.inverseMass = 1.0f / (m_mass.random(context.random));
		point.size = m_size.random(context.random);
		point.random = context.random.nextFloat();

		emitterInstance.addPoint(point);
	}
}

bool DiscSource::serialize(ISerializer& s)
{
	if (!Source::serialize(s))
		return false;

	s >> Member< Vector4 >(L"position", m_position);
	s >> Member< Vector4 >(L"normal", m_normal);
	s >> MemberComposite< Range< float > >(L"radius", m_radius);
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
