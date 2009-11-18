#ifndef traktor_mesh_StaticMeshEntityData_H
#define traktor_mesh_StaticMeshEntityData_H

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

class StaticMesh;

class T_DLLCLASS StaticMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< StaticMesh >& getMesh() const { return m_mesh; }

private:
	mutable resource::Proxy< StaticMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_StaticMeshEntityData_H
