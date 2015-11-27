#ifndef traktor_spray_DiscSource_H
#define traktor_spray_DiscSource_H

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

/*! \brief Disc particle source.
 * \ingroup Spray
 */
class T_DLLCLASS DiscSource : public Source
{
	T_RTTI_CLASS;

public:
	DiscSource(
		float constantRate,
		float velocityRate,
		const Vector4& position,
		const Vector4& normal,
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

	const Vector4& getNormal() const { return m_normal; }

	const Range< float >& getRadius() const { return m_radius; }

private:
	Vector4 m_position;
	Vector4 m_normal;
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

#endif	// traktor_spray_DiscSource_H
