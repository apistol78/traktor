#ifndef traktor_mesh_OctreePartition_H
#define traktor_mesh_OctreePartition_H

#include <map>
#include "Mesh/Partition/IPartition.h"

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

class OctreeNode;

class OctreePartition : public IPartition
{
	T_RTTI_CLASS;

public:
	OctreePartition(OctreeNode* node, const std::vector< render::handle_t >& worldTechniques);

	virtual void traverse(
		const Frustum& frustum,
		const Matrix44& worldView,
		render::handle_t worldTechnique,
		std::vector< uint32_t >& outPartIndices
	) const T_OVERRIDE T_FINAL;

private:
	Ref< OctreeNode > m_node;
	std::vector< render::handle_t > m_worldTechniques;
};

	}
}

#endif	// traktor_mesh_OctreePartition_H
