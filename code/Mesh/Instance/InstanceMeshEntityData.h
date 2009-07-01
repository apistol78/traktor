#ifndef traktor_mesh_InstanceMeshEntityData_H
#define traktor_mesh_InstanceMeshEntityData_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntityData.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#define T_DLLCLASS T_DLLEXPORT
#else
#define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

class T_DLLCLASS InstanceMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS(InstanceMeshEntityData)

public:
	virtual MeshEntity* createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< InstanceMesh >& getMesh() const { return m_mesh; }

private:
	mutable resource::Proxy< InstanceMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_InstanceMeshEntityData_H
