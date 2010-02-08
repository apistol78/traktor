#ifndef traktor_mesh_StreamMeshEntityData_H
#define traktor_mesh_StreamMeshEntityData_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntityData.h"

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

class StreamMesh;

class T_DLLCLASS StreamMeshEntityData : public MeshEntityData
{
	T_RTTI_CLASS;

public:
	virtual Ref< MeshEntity > createEntity(resource::IResourceManager* resourceManager, world::IEntityBuilder* builder) const;

	virtual bool serialize(ISerializer& s);

	inline const resource::Proxy< StreamMesh >& getMesh() const { return m_mesh; }

private:
	mutable resource::Proxy< StreamMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_StreamMeshEntityData_H
