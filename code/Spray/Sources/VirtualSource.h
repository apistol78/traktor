#ifndef traktor_spray_VirtualSource_H
#define traktor_spray_VirtualSource_H

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

/*! \brief Point particle source.
 * \ingroup Spray
 */
class T_DLLCLASS VirtualSource : public Source
{
	T_RTTI_CLASS;

public:
	VirtualSource(
		float constantRate,
		float velocityRate,
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

	const Range< float >& getVelocity() const { return m_velocity; }

	const Range< float >& getOrientation() const { return m_orientation; }

	const Range< float >& getAngularVelocity() const { return m_angularVelocity; }

	const Range< float >& getAge() const { return m_age; }

	const Range< float >& getMass() const { return m_mass; }

	const Range< float >& getSize() const { return m_size; }

private:
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_VirtualSource_H
