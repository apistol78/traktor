#include "Spray/EmitterInstance.h"
#include "Spray/Types.h"
#include "Spray/Sources/QuadSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.QuadSource", QuadSource, Source)

QuadSource::QuadSource(
	float constantRate,
	float velocityRate,
	const Vector4& center,
	const Vector4& axis1,
	const Vector4& axis2,
	const Vector4& normal,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_center(center.xyz1())
,	m_axis1(axis1.xyz0())
,	m_axis2(axis2.xyz0())
,	m_normal(normal.xyz0())
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void QuadSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	Vector4 center = transform * m_center;
	Vector4 axis1 = transform * m_axis1;
	Vector4 axis2 = transform * m_axis2;
	Vector4 normal = transform * m_normal;

	Point* point = emitterInstance.addPoints(emitCount);

	while (emitCount-- > 0)
	{
		Scalar u = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);
		Scalar v = Scalar(float(context.random.nextDouble()) * 2.0f - 1.0f);

		point->position = center + u * axis1 + v * axis2;
		point->velocity = normal * Scalar(m_velocity.random(context.random));
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

	}
}
