/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#ifndef traktor_mesh_OctreeNode_H
#define traktor_mesh_OctreeNode_H

#include <map>
#include <vector>
#include "Core/Object.h"
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

	namespace mesh
	{

class OctreeNodeData;

class T_DLLCLASS OctreeNode : public Object
{
	T_RTTI_CLASS;

public:
	OctreeNode(const OctreeNodeData* nodeData);

	void traverse(
		const Frustum& frustumObject,
		uint8_t worldTechniqueId,
		std::vector< uint32_t >& outPartIndices
	) const;

private:
	Aabb3 m_boundingBox;
	std::vector< std::vector< uint32_t > > m_partIndices;
	Ref< OctreeNode > m_children[8];
	bool m_leaf;
};

	}
}

#endif	// traktor_mesh_OctreeNode_H
