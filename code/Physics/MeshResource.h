#ifndef traktor_physics_MeshResource_H
#define traktor_physics_MeshResource_H

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

/*! \brief Mesh geometry resource.
 * \ingroup Physics
 */
class T_DLLCLASS MeshResource : public Serializable
{
	T_RTTI_CLASS(MeshResource)

public:
	virtual bool serialize(Serializer& s);
};

	}
}

#endif	// traktor_physics_MeshResource_H
