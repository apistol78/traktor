#pragma once

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

class PartitionMesh;

class T_DLLCLASS PartitionMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	PartitionMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< PartitionMesh >& mesh);

	virtual Aabb3 getBoundingBox() const override final;

	virtual bool supportTechnique(render::handle_t technique) const override final;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) override final;

private:
	resource::Proxy< PartitionMesh > m_mesh;
};

	}
}

