#include "Spray/Types.h"
#include "Spray/Sources/VirtualSource.h"

namespace traktor
{
	namespace spray
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.spray.VirtualSource", VirtualSource, Source)

VirtualSource::VirtualSource(
	float constantRate,
	float velocityRate,
	const Range< float >& velocity,
	const Range< float >& orientation,
	const Range< float >& angularVelocity,
	const Range< float >& age,
	const Range< float >& mass,
	const Range< float >& size
)
:	Source(constantRate, velocityRate)
,	m_velocity(velocity)
,	m_orientation(orientation)
,	m_angularVelocity(angularVelocity)
,	m_age(age)
,	m_mass(mass)
,	m_size(size)
{
}

void VirtualSource::emit(
	Context& context,
	const Transform& transform,
	const Vector4& deltaMotion,
	uint32_t emitCount,
	EmitterInstance& emitterInstance
) const
{
	if (context.virtualSourceCallback)
		context.virtualSourceCallback->virtualSourceEmit(
			this,
			context,
			transform,
			deltaMotion,
			emitCount,
			emitterInstance
		);
}

	}
}
