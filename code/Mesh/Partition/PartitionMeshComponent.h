#ifndef traktor_mesh_PartitionMeshComponent_H
#define traktor_mesh_PartitionMeshComponent_H

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"

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

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS PartitionMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	PartitionMeshComponent(world::Entity* owner, bool screenSpaceCulling, const resource::Proxy< PartitionMesh >& mesh);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass, const Transform& transform) T_OVERRIDE T_FINAL;

private:
	resource::Proxy< PartitionMesh > m_mesh;
};

	}
}

#endif	// traktor_mesh_PartitionMeshComponent_H
