#ifndef traktor_mesh_OctreeNode_H
#define traktor_mesh_OctreeNode_H

#include <map>
#include "Core/Object.h"
#include "Core/Math/Aabb.h"
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

class Frustum;
class Matrix44;

	namespace mesh
	{

class OctreeNodeData;

class T_DLLCLASS OctreeNode : public Object
{
	T_RTTI_CLASS;

public:
	OctreeNode(const OctreeNodeData* nodeData, const std::vector< render::handle_t >& worldTechniques);

	void traverse(
		const Frustum& frustumObject,
		render::handle_t worldTechnique,
		std::set< uint32_t >& outPartIndices
	) const;

private:
	Aabb m_boundingBox;
	std::map< render::handle_t, std::vector< uint32_t > > m_partIndices;
	Ref< OctreeNode > m_children[8];
};

	}
}

#endif	// traktor_mesh_OctreeNode_H
