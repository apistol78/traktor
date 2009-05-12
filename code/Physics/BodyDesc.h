#ifndef traktor_physics_BodyDesc_H
#define traktor_physics_BodyDesc_H

#include <string>
#include "Core/Heap/Ref.h"
#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_PHYSICS_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace physics
	{

class ShapeDesc;

/*! \brief Rigid body description.
 * \ingroup Physics
 */
class T_DLLCLASS BodyDesc : public Serializable
{
	T_RTTI_CLASS(BodyDesc)

public:
	void setShape(ShapeDesc* shape);

	const Ref< ShapeDesc >& getShape() const;

	virtual bool serialize(Serializer& s);

private:
	Ref< ShapeDesc > m_shape;
};

	}
}

#endif	// traktor_physics_BodyDesc_H
