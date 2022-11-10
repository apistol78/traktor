#include "Core/Math/Frustum.h"
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreeNodeData.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreeNode", OctreeNode, Object)

OctreeNode::OctreeNode(const OctreeNodeData* nodeData)
:	m_boundingBox(nodeData->m_boundingBox)
,	m_leaf(true)
{
	for (const auto& it : nodeData->m_partIndices)
	{
		if (it.first >= m_partIndices.size())
			m_partIndices.resize(it.first + 1);
		m_partIndices[it.first] = it.second;
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
	AlignedVector< uint32_t >& outPartIndices
) const
{
	Frustum::InsideResult result = frustumObject.inside(m_boundingBox);
	if (result == Frustum::IrInside)
	{
		if (worldTechniqueId < m_partIndices.size())
		{
			const auto& parts = m_partIndices[worldTechniqueId];
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
				const auto& parts = m_partIndices[worldTechniqueId];
				outPartIndices.insert(outPartIndices.end(), parts.begin(), parts.end());
			}
		}
	}
}

}
