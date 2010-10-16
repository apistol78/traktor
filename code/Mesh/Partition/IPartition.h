#ifndef traktor_mesh_IPartition_H
#define traktor_mesh_IPartition_H

#include "Core/Object.h"
#include "Core/Math/Frustum.h"
#include "Core/Math/Matrix44.h"
#include "Render/Types.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class IPartition : public Object
{
	T_RTTI_CLASS;

public:
	virtual void traverse(
		const Frustum& frustum,
		const Matrix44& world,
		const Matrix44& view,
		render::handle_t worldTechnique,
		std::vector< uint32_t >& outPartIndices
	) const = 0;
};

	}
}

#endif	// traktor_mesh_IPartition_H
