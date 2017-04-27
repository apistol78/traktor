/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include <algorithm>
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreePartition.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreePartition", OctreePartition, IPartition)

OctreePartition::OctreePartition(OctreeNode* node, const std::vector< render::handle_t >& worldTechniques)
:	m_node(node)
,	m_worldTechniques(worldTechniques)
{
}

void OctreePartition::traverse(
	const Frustum& frustum,
	const Matrix44& worldView,
	render::handle_t worldTechnique,
	std::vector< uint32_t >& outPartIndices
) const
{
	std::vector< render::handle_t >::const_iterator it = std::find(m_worldTechniques.begin(), m_worldTechniques.end(), worldTechnique);
	if (it == m_worldTechniques.end())
		return;

	uint8_t worldTechniqueId = uint8_t(std::distance(m_worldTechniques.begin(), it));

	Matrix44 worldViewInv = worldView.inverse();
	Frustum frustumObject;

	for (int32_t i = 0; i < 6; ++i)
		frustumObject.planes[i] = worldViewInv * frustum.planes[i];

	m_node->traverse(frustumObject, worldTechniqueId, outPartIndices);
}

	}
}
