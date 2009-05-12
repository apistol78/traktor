#ifndef traktor_mesh_MeshResource_H
#define traktor_mesh_MeshResource_H

#include "Core/Serialization/Serializable.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

/*! \brief Mesh resource.
 *
 * Base class for all mesh resources.
 */
class T_DLLCLASS MeshResource : public Serializable
{
	T_RTTI_CLASS(MeshResource)
};

	}
}

#endif	// traktor_mesh_MeshResource_H
