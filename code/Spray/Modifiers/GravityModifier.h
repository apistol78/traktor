#ifndef traktor_spray_GravityModifier_H
#define traktor_spray_GravityModifier_H

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

/*! \brief Gravity modifier.
 * \ingroup Spray
 */
class T_DLLCLASS GravityModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	GravityModifier();

	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;

	virtual bool serialize(ISerializer& s);

private:
	Vector4 m_gravity;
	bool m_world;
};

	}
}

#endif	// traktor_spray_GravityModifier_H
