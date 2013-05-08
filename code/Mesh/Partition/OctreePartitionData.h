#ifndef traktor_mesh_OctreePartitionData_H
#define traktor_mesh_OctreePartitionData_H

#include <map>
#include "Mesh/Partition/IPartitionData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class OctreeNodeData;

class T_DLLCLASS OctreePartitionData : public IPartitionData
{
	T_RTTI_CLASS;

public:
	virtual Ref< IPartition > createPartition() const;

	virtual void serialize(ISerializer& s);

private:
	friend class PartitionMeshConverter;

	std::vector< std::wstring > m_worldTechniques;
	Ref< OctreeNodeData > m_nodeData;
};

	}
}

#endif	// traktor_mesh_OctreePartitionData_H
