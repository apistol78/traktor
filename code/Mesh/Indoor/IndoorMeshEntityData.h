#ifndef traktor_mesh_IndoorMeshEntityData_H
#define traktor_mesh_IndoorMeshEntityData_H

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

class IndoorMesh;

class T_DLLCLASS IndoorMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS(IndoorMeshEntityData)

public:
	virtual MeshEntity* createEntity(world::EntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< IndoorMesh >& getMesh() const { return m_mesh; }

private:
	resource::Proxy< IndoorMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_IndoorMeshEntityData_H
