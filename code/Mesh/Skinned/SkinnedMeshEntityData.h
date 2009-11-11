#ifndef traktor_mesh_SkinnedMeshEntityData_H
#define traktor_mesh_SkinnedMeshEntityData_H

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

class SkinnedMesh;

class T_DLLCLASS SkinnedMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS(SkinnedMeshEntityData)

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(Serializer& s);

	inline const resource::Proxy< SkinnedMesh >& getMesh() const { return m_mesh; }

private:
	mutable resource::Proxy< SkinnedMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_SkinnedMeshEntityData_H
