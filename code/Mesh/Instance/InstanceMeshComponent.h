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

namespace traktor
{
	namespace mesh
	{

class InstanceMesh;

/*! Instancing mesh component.
 * \ingroup Mesh
 */
class T_DLLCLASS InstanceMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	InstanceMeshComponent(const resource::Proxy< InstanceMesh >& mesh, bool screenSpaceCulling);

	virtual void destroy() override final;

	virtual Aabb3 getBoundingBox() const override final;

	virtual void build(world::WorldContext& worldContext, const world::WorldRenderView& worldRenderView, const world::IWorldRenderPass& worldRenderPass) override final;

	inline resource::Proxy< InstanceMesh >& getMesh() { return m_mesh; }

private:
	resource::Proxy< InstanceMesh > m_mesh;
};

	}
}

