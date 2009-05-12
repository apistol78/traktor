#ifndef traktor_spray_PointSource_H
#define traktor_spray_PointSource_H

#include "Spray/Source.h"
#include "Spray/Range.h"
#include "Core/Math/RandomGeometry.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_SPRAY_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace spray
	{

/*! \brief Point particle source.
 * \ingroup Spray
 */
class T_DLLCLASS PointSource : public Source
{
	T_RTTI_CLASS(PointSource)

public:
	PointSource();

	virtual void emit(
		EmitterUpdateContext& context,
		const Matrix44& transform,
		uint32_t emitCount,
		EmitterInstance& emitterInstance
	) const;

	virtual bool serialize(Serializer& s);

	inline const Vector4& getPosition() const { return m_position; }

private:
	Vector4 m_position;
	Range< float > m_velocity;
	Range< float > m_orientation;
	Range< float > m_angularVelocity;
	Range< float > m_age;
	Range< float > m_mass;
	Range< float > m_size;
};

	}
}

#endif	// traktor_spray_PointSource_H
