#ifndef traktor_spray_DragModifier_H
#define traktor_spray_DragModifier_H

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

/*! \brief Drag modifier.
 * \ingroup Spray
 */
class T_DLLCLASS DragModifier : public Modifier
{
	T_RTTI_CLASS;

public:
	DragModifier();

	virtual void update(const Scalar& deltaTime, const Transform& transform, PointVector& points, size_t first, size_t last) const;

	virtual bool serialize(ISerializer& s);

private:
	Scalar m_linearDrag;
	float m_angularDrag;
};

	}
}

#endif	// traktor_spray_DragModifier_H
