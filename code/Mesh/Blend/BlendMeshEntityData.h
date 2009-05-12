#ifndef traktor_mesh_BlendMeshEntityData_H
#define traktor_mesh_BlendMeshEntityData_H

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

class BlendMesh;

class T_DLLCLASS BlendMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS(BlendMeshEntityData)

public:
	virtual MeshEntity* createEntity(world::EntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< BlendMesh >& getMesh() const { return m_mesh; }

private:
	resource::Proxy< BlendMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_BlendMeshEntityData_H
