#ifndef traktor_mesh_InstanceMeshEntityRenderer_H
#define traktor_mesh_InstanceMeshEntityRenderer_H

#include "Core/Containers/SmallMap.h"
#include "Mesh/Instance/InstanceMesh.h"
#include "Mesh/Instance/InstanceMeshData.h"
#include "World/Entity/IEntityRenderer.h"

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

class T_DLLCLASS InstanceMeshEntityRenderer : public world::IEntityRenderer
{
	T_RTTI_CLASS;

public:
	virtual const TypeInfoSet getEntityTypes() const;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		world::Entity* entity
	);

	virtual void flush(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass
	);

private:
	SmallMap< InstanceMesh*, AlignedVector< InstanceMesh::instance_distance_t > > m_meshInstances;
};

	}
}

#endif	// traktor_mesh_InstanceMeshEntityRenderer_H
