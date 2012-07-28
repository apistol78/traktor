#ifndef traktor_mesh_LodMeshEntity_H
#define traktor_mesh_LodMeshEntity_H

#include "Core/RefArray.h"
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

class T_DLLCLASS LodMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	LodMeshEntity(
		const Transform& transform,
		const RefArray< MeshEntity >& lods,
		float lodDistance,
		float lodCullDistance
	);
	
	virtual void setTransform(const Transform& transform);

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

	virtual void update(const UpdateParams& update);

private:
	RefArray< MeshEntity > m_lods;
	float m_lodDistance;
	float m_lodCullDistance;
	Aabb3 m_boundingBox;
};

	}
}

#endif	// traktor_mesh_LodMeshEntity_H
