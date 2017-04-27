/*
================================================================================================
CONFIDENTIAL AND PROPRIETARY INFORMATION/NOT FOR DISCLOSURE WITHOUT WRITTEN PERMISSION
Copyright 2017 Doctor Entertainment AB. All Rights Reserved.
================================================================================================
*/
#include "Core/Serialization/ISerializer.h"
#include "Core/Serialization/MemberRef.h"
#include "Core/Serialization/MemberStl.h"
#include "Mesh/Partition/OctreeNode.h"
#include "Mesh/Partition/OctreeNodeData.h"
#include "Mesh/Partition/OctreePartition.h"
#include "Mesh/Partition/OctreePartitionData.h"

namespace traktor
{
	namespace mesh
	{

T_IMPLEMENT_RTTI_FACTORY_CLASS(L"traktor.mesh.OctreePartitionData", 0, OctreePartitionData, IPartitionData)

Ref< IPartition > OctreePartitionData::createPartition() const
{
	std::vector< render::handle_t > worldTechniques(m_worldTechniques.size());
	for (uint32_t i = 0; i < m_worldTechniques.size(); ++i)
		worldTechniques[i] = render::getParameterHandle(m_worldTechniques[i]);
	return new OctreePartition(new OctreeNode(m_nodeData), worldTechniques);
}

void OctreePartitionData::serialize(ISerializer& s)
{
	s >> MemberStlVector< std::wstring >(L"worldTechniques", m_worldTechniques);
	s >> MemberRef< OctreeNodeData >(L"nodeData", m_nodeData);
}

	}
}
