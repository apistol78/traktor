#pragma once

#include "Core/Object.h"
#include "Core/Containers/AlignedVector.h"
#include "Core/Math/Aabb3.h"
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

}

namespace traktor::mesh
{

class OctreeNodeData;

/*!
 * \ingroup Mesh
 */
class T_DLLCLASS OctreeNode : public Object
{
	T_RTTI_CLASS;

public:
	explicit OctreeNode(const OctreeNodeData* nodeData);

	void traverse(
		const Frustum& frustumObject,
		uint8_t worldTechniqueId,
		AlignedVector< uint32_t >& outPartIndices
	) const;

private:
	Aabb3 m_boundingBox;
	AlignedVector< AlignedVector< uint32_t > > m_partIndices;
	Ref< OctreeNode > m_children[8];
	bool m_leaf;
};

}
