#ifndef traktor_physics_CompoundShapeDesc_H
#define traktor_physics_CompoundShapeDesc_H

#include "Core/RefArray.h"
#include "Physics/ShapeDesc.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

/*! \brief Compound collision shape.
 * \ingroup Physics
 */
class T_DLLCLASS CompoundShapeDesc : public ShapeDesc
{
	T_RTTI_CLASS;

public:
	virtual void serialize(ISerializer& s) T_OVERRIDE T_FINAL;

private:
	RefArray< ShapeDesc > m_shapes;
};

	}
}

#endif	// traktor_physics_CompoundShapeDesc_H
