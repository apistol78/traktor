#ifndef traktor_mesh_BlendMeshComponent_H
#define traktor_mesh_BlendMeshComponent_H

#include "Mesh/MeshComponent.h"
#include "Mesh/Blend/BlendMesh.h"
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

/*! \brief
 * \ingroup Mesh
 */
class T_DLLCLASS BlendMeshComponent : public MeshComponent
{
	T_RTTI_CLASS;

public:
	BlendMeshComponent(world::Entity* owner, bool screenSpaceCulling, const resource::Proxy< BlendMesh >& mesh);

	virtual void destroy() T_OVERRIDE T_FINAL;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual void render(world::WorldContext& worldContext, world::WorldRenderView& worldRenderView, world::IWorldRenderPass& worldRenderPass) T_OVERRIDE T_FINAL;

	void setBlendWeights(const std::vector< float >& blendWeights);

	const std::vector< float >& getBlendWeights() const;

private:
	resource::Proxy< BlendMesh > m_mesh;
	Ref< BlendMesh::Instance > m_instance;
	std::vector< float > m_blendWeights;
};

	}
}

#endif	// traktor_mesh_BlendMeshComponent_H
