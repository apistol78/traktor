#ifndef traktor_mesh_BlendMeshEntity_H
#define traktor_mesh_BlendMeshEntity_H

#include "Mesh/MeshEntity.h"
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

/*! \brief Blend mesh entity
 * \ingroup Mesh
 */
class T_DLLCLASS BlendMeshEntity : public MeshEntity
{
	T_RTTI_CLASS;

public:
	BlendMeshEntity(const Transform& transform, bool screenSpaceCulling, const resource::Proxy< BlendMesh >& mesh);

	void setBlendWeights(const std::vector< float >& blendWeights);

	const std::vector< float >& getBlendWeights() const;

	virtual Aabb3 getBoundingBox() const T_OVERRIDE T_FINAL;

	virtual bool supportTechnique(render::handle_t technique) const T_OVERRIDE T_FINAL;

	virtual void render(
		world::WorldContext& worldContext,
		world::WorldRenderView& worldRenderView,
		world::IWorldRenderPass& worldRenderPass,
		float distance
	) T_OVERRIDE T_FINAL;

private:
	resource::Proxy< BlendMesh > m_mesh;
	Ref< BlendMesh::Instance > m_instance;
	std::vector< float > m_blendWeights;
};

	}
}

#endif	// traktor_mesh_BlendMeshEntity_H
