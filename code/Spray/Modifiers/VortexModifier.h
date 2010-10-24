#ifndef traktor_spray_VortexModifier_H
#define traktor_spray_VortexModifier_H

#include "Spray/Modifier.h"

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

/*! \brief Vortex modifier.
 * \ingroup Spray
 */
class T_DLLCLASS VortexModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	VortexModifier();

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;
#endif

	virtual bool serialize(ISerializer& s);

private:
	Vector4 m_axis;
	Scalar m_tangentForce;			//< Amount of force applied to each particle in tangent direction.
	Scalar m_normalConstantForce;	//< Amount of constant force applied to each particle in normal direction.
	Scalar m_normalDistance;		//< Distance from axis for each particle, scaled together with m_normalDistanceForce in order to apply different amount of force based on distance.
	Scalar m_normalDistanceForce;
	bool m_world;
};

	}
}

#endif	// traktor_spray_VortexModifier_H
