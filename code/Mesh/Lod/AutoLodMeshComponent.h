#ifndef traktor_mesh_AutoLodMeshComponent_H
#define traktor_mesh_AutoLodMeshComponent_H

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

class AutoLodMesh;

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS AutoLodMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	AutoLodMeshComponent(world::Entity* owner, bool screenSpaceCulling, const resource::Proxy< AutoLodMesh >& mesh);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass) T_OVERRIDE T_FINAL;

private:
	resource::Proxy< AutoLodMesh > m_mesh;
	float m_lodDistance;
};

	}
}

#endif	// traktor_mesh_AutoLodMeshComponent_H
