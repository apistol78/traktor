/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Log/Log.h"
#include "Core/Math/Frustum.h"
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreeNodeData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreeNode", OctreeNode, Object)

OctreeNode::OctreeNode(const OctreeNodeData* nodeData)
:	m_boundingBox(nodeData->m_boundingBox)
,	m_leaf(true)
{
	for (std::map< uint8_t, std::vector< uint32_t > >::const_iterator i = nodeData->m_partIndices.begin(); i != nodeData->m_partIndices.end(); ++i)
	{
		if (i->first >= m_partIndices.size())
			m_partIndices.resize(i->first + 1);
		m_partIndices[i->first] = i->second;
	}

	for (int32_t i = 0; i < 8; ++i)
	{
		if (nodeData->m_children[i])
		{
			m_children[i] = new OctreeNode(nodeData->m_children[i]);
			m_leaf = false;
		}
	}
}

void OctreeNode::traverse(
	const Frustum& frustumObject,
	uint8_t worldTechniqueId,
	std::vector< uint32_t >& outPartIndices
) const
{
	Frustum::InsideResult result = frustumObject.inside(m_boundingBox);
	if (result == Frustum::IrInside)
	{
		if (worldTechniqueId < m_partIndices.size())
		{
			const std::vector< uint32_t >& parts = m_partIndices[worldTechniqueId];
			outPartIndices.insert(outPartIndices.end(), parts.begin(), parts.end());
		}
	}
	else if (result == Frustum::IrPartial)
	{
		if (!m_leaf)
		{
			for (int32_t i = 0; i < 8; ++i)
			{
				if (m_children[i])
					m_children[i]->traverse(frustumObject, worldTechniqueId, outPartIndices);
			}
		}
		else
		{
			if (worldTechniqueId < m_partIndices.size())
			{
				const std::vector< uint32_t >& parts = m_partIndices[worldTechniqueId];
				outPartIndices.insert(outPartIndices.end(), parts.begin(), parts.end());
			}
		}
	}
}

	}
}
