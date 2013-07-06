#ifndef traktor_mesh_InstanceMeshEntity_H
#define traktor_mesh_InstanceMeshEntity_H

#include "Resource/Proxy.h"
#include "Mesh/MeshEntity.h"

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

class InstanceMesh;

class T_DLLCLASS InstanceMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	InstanceMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< InstanceMesh >& mesh);
	
	virtual Aabb3 getBoundingBox() const;

	virtual bool supportTechnique(render::handle_t technique) const;

	virtual void precull(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView
	);

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	);

	inline resource::Proxy< InstanceMesh >& getMesh() { return m_mesh; }

private:
	friend class InstanceMeshEntityRenderer;

	resource::Proxy< InstanceMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_InstanceMeshEntity_H
