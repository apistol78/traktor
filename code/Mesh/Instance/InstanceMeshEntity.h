#ifndef traktor_mesh_InstanceMeshEntity_H
#define traktor_mesh_InstanceMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

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

class T_DLLCLASS InstanceMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	InstanceMeshEntity(const Transform& transform, const resource::Proxy< InstanceMesh >& mesh);
	
	virtual Aabb getBoundingBox() const;

	virtual void update(const world::EntityUpdate* update);

	virtual void render(world::WorldContext* worldContext, world::WorldRenderView* worldRenderView, float distance);

	inline resource::Proxy< InstanceMesh >& getMesh() { return m_mesh; }

private:
	mutable resource::Proxy< InstanceMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_InstanceMeshEntity_H
