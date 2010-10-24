#ifndef traktor_spray_SizeModifier_H
#define traktor_spray_SizeModifier_H

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

/*! \brief Particle size modifier.
 * \ingroup Spray
 */
class T_DLLCLASS SizeModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	SizeModifier();

#if defined(T_MODIFIER_USE_PS3_SPURS)
	virtual void update(SpursJobQueue* jobQueue, const Scalar& deltaTime, const Transform& transform, PointVector& points) const;
#else
	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;
#endif

	virtual bool serialize(ISerializer& s);

private:
	float m_adjustRate;
};

	}
}

#endif	// traktor_spray_SizeModifier_H
