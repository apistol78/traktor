#ifndef traktor_mesh_MeshResource_H
#define traktor_mesh_MeshResource_H

#include "Core/Serialization/ISerializable.h"

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
class T_DLLCLASS MeshResource : public ISerializable
{
	T_RTTI_CLASS;
};

	}
}

#endif	// traktor_mesh_MeshResource_H
