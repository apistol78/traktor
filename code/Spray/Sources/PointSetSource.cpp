#include "Spray/EmitterInstance.h"
#include "Spray/Types.h"
#include "Spray/PointSet.h"
#include "Spray/Sources/PointSetSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.PointSetSource", PointSetSource, Source)

PointSetSource::PointSetSource(
	float constantRate,
	float velocityRate,
	const resource::Proxy< PointSet >& pointSet,
	const Vector4& offset,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_pointSet(pointSet)
,	m_offset(offset)
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void PointSetSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	const AlignedVector< PointSet::Point >& points = m_pointSet->get();
	if (points.empty())
		return;

	Point* point = emitterInstance.addPoints(uint32_t(points.size()));

	for (uint32_t i = 0; i < uint32_t(points.size()); ++i)
	{
		Vector4 position = transform * (points[i].position + m_offset).xyz1();
		Vector4 normal = transform.rotation() * points[i].normal;

		point->position = position;
		point->velocity = normal * Scalar(m_velocity.random(context.random));
		point->orientation = m_orientation.random(context.random);
		point->angularVelocity = m_angularVelocity.random(context.random);
		point->color = points[i].color;
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
