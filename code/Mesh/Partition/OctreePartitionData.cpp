#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberAlignedVector.h"
#include "Core/Serialization/MemberRef.h"
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreeNodeData.h"
#include "Mesh/Partition/OctreePartition.h"
#include "Mesh/Partition/OctreePartitionData.h"

namespace traktor::mesh
{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.OctreePartitionData", 0, OctreePartitionData, IPartitionData)

Ref< IPartition > OctreePartitionData::createPartition() const
{
	AlignedVector< render::handle_t > worldTechniques(m_worldTechniques.size());
	for (uint32_t i = 0; i < m_worldTechniques.size(); ++i)
		worldTechniques[i] = render::getParameterHandle(m_worldTechniques[i]);
	return new OctreePartition(new OctreeNode(m_nodeData), worldTechniques);
}

void OctreePartitionData::serialize(ISerializer& s)
{
	s >> MemberAlignedVector< std::wstring >(L"worldTechniques", m_worldTechniques);
	s >> MemberRef< const OctreeNodeData >(L"nodeData", m_nodeData);
}

}
