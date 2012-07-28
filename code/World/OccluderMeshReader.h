#ifndef traktor_world_OccluderMeshReader_H
#define traktor_world_OccluderMeshReader_H

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

/*! \brief Occluder mesh reader.
 * \ingroup World
 */
class T_DLLCLASS OccluderMeshReader : public Object
{
	T_RTTI_CLASS;

public:
	Ref< OccluderMesh > read(IStream* stream) const;
};

	}
}

#endif	// traktor_world_OccluderMeshReader_H
