#ifndef traktor_spray_SphereSource_H
#define traktor_spray_SphereSource_H

#include "Core/Math/RandomGeometry.h"
#include "Core/Math/Range.h"
#include "Spray/Source.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Sphere particle source.
 * \ingroup Spray
 */
class T_DLLCLASS SphereSource : public Source
{
	T_RTTI_CLASS;

public:
	SphereSource(
		float constantRate,
		float velocityRate,
		const Vector4& position,
		const Range< float >& radius,
		const Range< float >& velocity,
		const Range< float >& orientation,
		const Range< float >& angularVelocity,
		const Range< float >& age,
		const Range< float >& mass,
		const Range< float >& size
	);

	virtual void emit(
		Context& context,
		const Transform& transform,
		const Vector4& deltaMotion,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const T_OVERRIDE T_FINAL;

	const Vector4& getPosition() const { return m_position; }

	const Range< float >& getRadius() const { return m_radius; }

private:
	Vector4 m_position;
	Range< float > m_radius;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_SphereSource_H
