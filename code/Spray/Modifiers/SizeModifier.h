#ifndef traktor_spray_SizeModifier_H
#define traktor_spray_SizeModifier_H

#include "Spray/Modifier.h"

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

/*! \brief Particle size modifier.
 * \ingroup Spray
 */
class T_DLLCLASS SizeModifier : public Modifier
{
	T_RTTI_CLASS(SizeModifier)

public:
	SizeModifier();

	virtual void update(const Scalar& deltaTime, const Matrix44& transform, PointVector& points, size_t first, size_t last) const;

	virtual bool serialize(Serializer& s);

private:
	float m_adjustRate;
};

	}
}

#endif	// traktor_spray_SizeModifier_H
