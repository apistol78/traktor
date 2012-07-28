#ifndef traktor_world_OccluderMeshWriter_H
#define traktor_world_OccluderMeshWriter_H

#include "Core/Object.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_WORLD_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{

class IStream;

	namespace world
	{

class OccluderMesh;

/*! \brief Occluder mesh writer.
 * \ingroup World
 */
class T_DLLCLASS OccluderMeshWriter : public Object
{
	T_RTTI_CLASS;

public:
	bool write(IStream* stream, const OccluderMesh* mesh) const;
};

	}
}

#endif	// traktor_world_OccluderMeshWriter_H
