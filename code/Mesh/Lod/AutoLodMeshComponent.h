#pragma once

#include "Mesh/MeshComponent.h"
#include "Resource/Proxy.h"

// import/export mechanism.
#undef T_DLLCLASS
#if defined(T_MESH_EXPORT)
#	define T_DLLCLASS T_DLLEXPORT
#else
#	define T_DLLCLASS T_DLLIMPORT
#endif

namespace traktor::mesh
{

class AutoLodMesh;

/*! Automatic LOD mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS AutoLodMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	explicit AutoLodMeshComponent(const resource::Proxy< AutoLodMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(const world::WorldBuildContext& context, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

private:
	resource::Proxy< AutoLodMesh > m_mesh;
	float m_lodDistance;
};

}
