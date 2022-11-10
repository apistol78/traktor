#pragma once

#include "Mesh/Partition/IPartition.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class OctreeNode;

/*!
 * \ingroup Mesh
 */
class OctreePartition : public IPartition
{
	T_RTTI_CLASS;

public:
	explicit OctreePartition(OctreeNode* node, const AlignedVector< render::handle_t >& worldTechniques);

	virtual void traverse(
		const Frustum& frustum,
		const Matrix44& worldView,
		render::handle_t worldTechnique,
		AlignedVector< uint32_t >& outPartIndices
	) const override final;

private:
	Ref< OctreeNode > m_node;
	AlignedVector< render::handle_t > m_worldTechniques;
};

}
