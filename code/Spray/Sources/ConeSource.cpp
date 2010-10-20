#include "Spray/Sources/ConeSource.h"
#include "Spray/EmitterUpdateContext.h"
#include "Spray/EmitterInstance.h"
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/Member.h"
#include "Core/Serialization/MemberComposite.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.spray.ConeSource", 0, ConeSource, Source)

ConeSource::ConeSource()
:	m_position(0.0f, 0.0f, 0.0f, 1.0f)
,	m_normal(0.0f, 1.0f, 0.0f, 0.0f)
,	m_angle1(PI)
,	m_angle2(PI)
{
}

bool ConeSource::bind(resource::IResourceManager* resourceManager)
{
	return true;
}

void ConeSource::emit(
	EmitterUpdateContext& context,
	const Transform& transform,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	Vector4 position = transform * m_position;

	Scalar wx = Scalar(sinf(m_angle1));
	Scalar wz = Scalar(sinf(m_angle2));
	
	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		float phi = context.random.nextFloat() * 2.0f * PI;
		Scalar gamma = Scalar(context.random.nextFloat());

		Scalar x = Scalar(cosf(phi));
		Scalar z = Scalar(sinf(phi));

		Vector4 extent = transform.axisX() * wx * x + transform.axisZ() * wz * z;
		Vector4 direction = (m_normal + extent * gamma).normalized();

		point->position = position;
		point->velocity = direction * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->color = Vector4::one();
		point->age = 0.0f;
		point->maxAge = m_age.random(context.random);
		point->inverseMass = 1.0f / (m_mass.random(context.random));
		point->size = m_size.random(context.random);
		point->random = context.random.nextFloat();
		
		++point;
	}
}

bool ConeSource::serialize(ISerializer& s)
{
	if (!Source::serialize(s))
		return false;

	s >> Member< Vector4 >(L"position", m_position);
	s >> Member< Vector4 >(L"normal", m_normal);
	s >> Member< float >(L"angle1", m_angle1);
	s >> Member< float >(L"angle2", m_angle2);
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
