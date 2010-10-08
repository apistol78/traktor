#include "Core/Log/Log.h"
#include "Core/Math/Frustum.h"
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreeNodeData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_CLASS(L"traktor.mesh.OctreeNode", OctreeNode, Object)

OctreeNode::OctreeNode(const OctreeNodeData* nodeData, const std::vector< render::handle_t >& worldTechniques)
:	m_boundingBox(nodeData->m_boundingBox)
{
	for (std::map< uint8_t, std::vector< uint32_t > >::const_iterator i = nodeData->m_partIndices.begin(); i != nodeData->m_partIndices.end(); ++i)
		m_partIndices[worldTechniques[i->first]] = i->second;

	for (int32_t i = 0; i < 8; ++i)
	{
		if (nodeData->m_children[i])
			m_children[i] = new OctreeNode(nodeData->m_children[i], worldTechniques);
	}
}

void OctreeNode::traverse(
	const Frustum& frustumObject,
	render::handle_t worldTechnique,
	std::set< uint32_t >& outPartIndices
) const
{
	Frustum::InsideResult result = frustumObject.inside(m_boundingBox);
	if (result == Frustum::IrInside)
	{
		std::map< render::handle_t, std::vector< uint32_t > >::const_iterator i = m_partIndices.find(worldTechnique);
		if (i != m_partIndices.end())
			outPartIndices.insert(i->second.begin(), i->second.end());
	}
	else if (result == Frustum::IrPartial)
	{
		bool leaf = true;
		for (int32_t i = 0; i < 8; ++i)
		{
			if (m_children[i])
			{
				m_children[i]->traverse(frustumObject, worldTechnique, outPartIndices);
				leaf = false;
			}
		}
		if (leaf)
		{
			std::map< render::handle_t, std::vector< uint32_t > >::const_iterator i = m_partIndices.find(worldTechnique);
			if (i != m_partIndices.end())
				outPartIndices.insert(i->second.begin(), i->second.end());
		}
	}
}

	}
}
